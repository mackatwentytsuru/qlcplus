#!/usr/bin/env python3
"""
MCP server for Mackie Control Universal (MCU) protocol.

Wraps the MCU MIDI protocol as Model Context Protocol tools, enabling AI
assistants to control physical MCU devices (Behringer X-TOUCH, MCU Pro, etc.).

Usage:
    python3 server.py                   # Run via stdio transport
    python3 -m mcp dev server.py        # Run with MCP inspector

Claude Code config (~/.claude.json):
    {
        "mcpServers": {
            "mcu": {
                "command": "python3",
                "args": ["/path/to/server.py"]
            }
        }
    }
"""

import json
import threading
import time
from collections import deque
from collections.abc import AsyncIterator
from contextlib import asynccontextmanager

import rtmidi
from mcp.server.fastmcp import Context, FastMCP

from mcu_protocol import (
    BUTTON_NAMES,
    CC_7SEG_BASE,
    CC_VPOT_LED_BASE,
    DEVICE_ID_MCU,
    NOTE_TO_NAME,
    SYSEX_CMD_CHALLENGE,
    SYSEX_CMD_CONFIRM,
    SYSEX_CMD_QUERY,
    SYSEX_CMD_RESPONSE,
    SYSEX_HEADER,
    build_sysex,
    compute_challenge_response,
    decode_midi_input,
    encode_vpot_led,
    value_to_pitch_bend,
)


# ---------------------------------------------------------------------------
# MCU Device state
# ---------------------------------------------------------------------------

class MCUDevice:
    """Manages MIDI connection and state for an MCU device."""

    def __init__(self):
        self.midi_out = rtmidi.MidiOut()
        self.midi_in = rtmidi.MidiIn()
        self.midi_in.ignore_types(sysex=False, timing=True, active_sense=True)

        self.device_id = DEVICE_ID_MCU
        self.handshake_state = "disconnected"
        self.port_name = ""
        self.serial_number: list[int] = []

        # State tracking
        self.fader_values = [0] * 9  # ch 0-7 + master
        self.input_buffer: deque[dict] = deque(maxlen=100)
        self._lock = threading.Lock()

    def send(self, message: list[int]):
        """Send a MIDI message."""
        if not self.midi_out.is_port_open():
            raise RuntimeError("MIDI output port not open")
        self.midi_out.send_message(message)

    def send_sysex(self, device_id: int, command: int, data: list[int]):
        """Send an MCU SysEx message."""
        self.send(build_sysex(device_id, command, data))

    def _midi_callback(self, event, _data=None):
        """Callback for incoming MIDI messages (runs in rtmidi thread)."""
        message, _delta = event

        # Handle SysEx for handshake
        if message and message[0] == 0xF0:
            self._handle_sysex(message)

        # Decode and buffer the input event
        decoded = decode_midi_input(message)
        if decoded:
            # Track fader state
            if decoded["type"] == "fader":
                self.fader_values[decoded["channel"]] = decoded["value"]

            with self._lock:
                self.input_buffer.append(decoded)

    def _handle_sysex(self, data: list[int]):
        """Process incoming SysEx for MCU handshake."""
        # Minimum: F0 00 00 66 [deviceId] [cmd] ... F7
        if len(data) < 7:
            return

        # Verify Mackie header
        if data[1:4] != SYSEX_HEADER:
            return

        dev_id = data[4]
        cmd = data[5]

        if dev_id != self.device_id:
            return

        if cmd == SYSEX_CMD_CHALLENGE:
            # Challenge: F0 00 00 66 [id] 01 [serial x7] [challenge x4] F7
            if len(data) < 18:
                return
            if self.handshake_state not in ("waiting_for_challenge", "disconnected"):
                return

            self.serial_number = data[6:13]
            challenge = data[13:17]
            response = compute_challenge_response(challenge)

            # Send response: F0 00 00 66 [id] 02 [serial x7] [response x4] F7
            self.send_sysex(self.device_id, SYSEX_CMD_RESPONSE,
                            self.serial_number + response)
            self.handshake_state = "waiting_for_confirm"

        elif cmd == SYSEX_CMD_CONFIRM:
            self.handshake_state = "connected"

    def close(self):
        """Close MIDI ports."""
        if self.midi_out.is_port_open():
            self.midi_out.close_port()
        if self.midi_in.is_port_open():
            self.midi_in.close_port()
        self.handshake_state = "disconnected"


# ---------------------------------------------------------------------------
# MCP Server lifespan
# ---------------------------------------------------------------------------

@asynccontextmanager
async def lifespan(_server: FastMCP) -> AsyncIterator[MCUDevice]:
    device = MCUDevice()
    try:
        yield device
    finally:
        device.close()


mcp = FastMCP("MCU Controller", lifespan=lifespan)


def _dev(ctx: Context) -> MCUDevice:
    return ctx.request_context.lifespan_context


# ---------------------------------------------------------------------------
# Connection tools
# ---------------------------------------------------------------------------

@mcp.tool()
def list_midi_ports(ctx: Context) -> str:
    """List all available MIDI input and output ports."""
    dev = _dev(ctx)
    result = {
        "output_ports": [
            {"index": i, "name": n}
            for i, n in enumerate(dev.midi_out.get_ports())
        ],
        "input_ports": [
            {"index": i, "name": n}
            for i, n in enumerate(dev.midi_in.get_ports())
        ],
    }
    return json.dumps(result, indent=2)


@mcp.tool()
def connect(ctx: Context, output_port: int, input_port: int = -1) -> str:
    """Connect to an MCU device and initiate handshake.

    Args:
        output_port: Output port index (from list_midi_ports)
        input_port: Input port index. -1 to skip input (output-only mode)
    """
    dev = _dev(ctx)

    # Close any existing connection
    dev.close()

    out_ports = dev.midi_out.get_ports()
    if output_port < 0 or output_port >= len(out_ports):
        return json.dumps({"error": f"Invalid output port {output_port}. "
                          f"Available: 0-{len(out_ports)-1}"})

    dev.midi_out.open_port(output_port)
    dev.port_name = out_ports[output_port]

    if input_port >= 0:
        in_ports = dev.midi_in.get_ports()
        if input_port >= len(in_ports):
            return json.dumps({"error": f"Invalid input port {input_port}. "
                              f"Available: 0-{len(in_ports)-1}"})
        dev.midi_in.open_port(input_port)
        dev.midi_in.set_callback(dev._midi_callback)

    # Initiate MCU handshake
    dev.handshake_state = "waiting_for_challenge"
    dev.send_sysex(dev.device_id, SYSEX_CMD_QUERY, [])

    # Wait briefly for handshake to complete
    for _ in range(10):
        time.sleep(0.1)
        if dev.handshake_state == "connected":
            break

    return json.dumps({
        "status": dev.handshake_state,
        "port": dev.port_name,
    })


@mcp.tool()
def disconnect(ctx: Context) -> str:
    """Disconnect from the MCU device."""
    dev = _dev(ctx)
    dev.close()
    return json.dumps({"status": "disconnected"})


# ---------------------------------------------------------------------------
# Fader tools
# ---------------------------------------------------------------------------

@mcp.tool()
def set_fader(ctx: Context, channel: int, value: int) -> str:
    """Set a fader position.

    Args:
        channel: Fader number 1-8 for channel faders, 9 for master
        value: Position 0 (bottom) to 255 (top)
    """
    dev = _dev(ctx)
    ch = channel - 1
    if not 0 <= ch <= 8:
        return json.dumps({"error": "Channel must be 1-9"})
    if not 0 <= value <= 255:
        return json.dumps({"error": "Value must be 0-255"})

    lsb, msb = value_to_pitch_bend(value)
    dev.send([0xE0 | ch, lsb, msb])
    dev.fader_values[ch] = value
    return json.dumps({"channel": channel, "value": value})


@mcp.tool()
def set_faders(ctx: Context, values: str) -> str:
    """Set multiple faders at once.

    Args:
        values: JSON object mapping channel numbers (1-9) to values (0-255).
                Example: '{"1": 200, "3": 128, "9": 255}'
    """
    dev = _dev(ctx)
    try:
        mapping = json.loads(values)
    except json.JSONDecodeError:
        return json.dumps({"error": "Invalid JSON"})

    results = []
    for ch_str, val in mapping.items():
        ch = int(ch_str) - 1
        if not 0 <= ch <= 8 or not 0 <= val <= 255:
            continue
        lsb, msb = value_to_pitch_bend(val)
        dev.send([0xE0 | ch, lsb, msb])
        dev.fader_values[ch] = val
        results.append({"channel": ch + 1, "value": val})

    return json.dumps({"set": results})


# ---------------------------------------------------------------------------
# Button / LED tools
# ---------------------------------------------------------------------------

@mcp.tool()
def set_button_led(ctx: Context, name: str, state: str = "on") -> str:
    """Set a button LED on, off, or blinking.

    Args:
        name: Button name (e.g. "play", "stop", "rec1", "solo3", "mute5", "f1").
              Use list_buttons to see all available names.
        state: "on", "off", or "blink"
    """
    dev = _dev(ctx)
    note = BUTTON_NAMES.get(name.lower())
    if note is None:
        return json.dumps({
            "error": f"Unknown button: {name}",
            "available": sorted(BUTTON_NAMES.keys()),
        })

    vel_map = {"on": 0x7F, "off": 0x00, "blink": 0x01}
    vel = vel_map.get(state.lower())
    if vel is None:
        return json.dumps({"error": "State must be 'on', 'off', or 'blink'"})

    dev.send([0x90, note, vel])
    return json.dumps({"button": name, "led": state})


@mcp.tool()
def list_buttons() -> str:
    """List all available MCU button names grouped by category."""
    categories = {
        "per_channel": [f"{t}{i}" for t in ["rec", "solo", "mute", "select"]
                        for i in range(1, 9)],
        "transport": ["rewind", "forward", "stop", "play", "record", "cycle"],
        "function": [f"f{i}" for i in range(1, 9)],
        "navigation": ["bank_left", "bank_right", "ch_left", "ch_right"],
        "cursor": ["up", "down", "left", "right", "zoom", "scrub"],
        "modifiers": ["shift", "option", "control", "alt"],
        "automation": ["read", "write", "trim", "touch", "latch", "group"],
        "utility": ["save", "undo", "cancel", "enter"],
        "assignment": ["track", "send", "pan", "plugin", "eq", "instrument"],
        "other": ["flip", "global_view", "marker", "nudge", "drop",
                  "replace", "click", "solo_defeat", "user_a", "user_b"],
    }
    return json.dumps(categories, indent=2)


# ---------------------------------------------------------------------------
# VPot LED ring tool
# ---------------------------------------------------------------------------

@mcp.tool()
def set_vpot_ring(ctx: Context, channel: int, value: int = 0,
                  mode: str = "single", center_led: bool = False) -> str:
    """Set a VPot LED ring display.

    Args:
        channel: VPot number 1-8
        value: LED position 0-11 (0=off, 1=leftmost, 6=center, 11=rightmost)
        mode: "single" (dot), "boost_cut" (from center), "wrap" (bar from left),
              "spread" (from center outward)
        center_led: Light the center LED
    """
    dev = _dev(ctx)
    ch = channel - 1
    if not 0 <= ch <= 7:
        return json.dumps({"error": "Channel must be 1-8"})

    mode_map = {"single": 0, "boost_cut": 1, "wrap": 2, "spread": 3}
    mode_val = mode_map.get(mode.lower())
    if mode_val is None:
        return json.dumps({"error": f"Unknown mode: {mode}. "
                          "Use: single, boost_cut, wrap, spread"})

    encoded = encode_vpot_led(value, mode_val, center_led)
    dev.send([0xB0, CC_VPOT_LED_BASE + ch, encoded])
    return json.dumps({"channel": channel, "value": value,
                       "mode": mode, "center_led": center_led})


# ---------------------------------------------------------------------------
# LCD display tools
# ---------------------------------------------------------------------------

@mcp.tool()
def write_lcd(ctx: Context, line: int, channel: int, text: str) -> str:
    """Write text to one channel position on the LCD scribble strip.

    Args:
        line: Line number 1 (top) or 2 (bottom)
        channel: Channel position 1-8 (each gets 7 characters)
        text: Text to display (max 7 chars, auto-padded with spaces)
    """
    dev = _dev(ctx)
    if line not in (1, 2):
        return json.dumps({"error": "Line must be 1 or 2"})
    ch = channel - 1
    if not 0 <= ch <= 7:
        return json.dumps({"error": "Channel must be 1-8"})

    offset = ((line - 1) * 0x38) + (ch * 7)
    padded = text.ljust(7)[:7]
    data = [offset] + [ord(c) & 0x7F for c in padded]
    dev.send_sysex(dev.device_id, SYSEX_CMD_LCD, data)
    return json.dumps({"line": line, "channel": channel, "text": padded})


@mcp.tool()
def write_lcd_full(ctx: Context, line1: str = "", line2: str = "") -> str:
    """Write full text to both LCD lines at once.

    Args:
        line1: Top line text (max 56 chars, 7 per channel × 8 channels)
        line2: Bottom line text (max 56 chars)
    """
    dev = _dev(ctx)

    for line_num, text in [(0, line1), (1, line2)]:
        if text:
            padded = text.ljust(56)[:56]
            offset = line_num * 0x38
            data = [offset] + [ord(c) & 0x7F for c in padded]
            dev.send_sysex(dev.device_id, SYSEX_CMD_LCD, data)

    return json.dumps({"line1": line1[:56], "line2": line2[:56]})


# ---------------------------------------------------------------------------
# 7-Segment / Timecode display tool
# ---------------------------------------------------------------------------

@mcp.tool()
def set_timecode(ctx: Context, text: str) -> str:
    """Set the 7-segment timecode display.

    Args:
        text: Text to display (max 12 chars, right-justified).
              Supports digits, some letters, and spaces.
    """
    dev = _dev(ctx)
    padded = text.rjust(12)[-12:]

    for i in range(12):
        ch = padded[11 - i]
        display_char = ord(ch) & 0x7F
        if display_char < 0x20:
            display_char = 0x20
        dev.send([0xB0, CC_7SEG_BASE + i, display_char])

    return json.dumps({"text": padded})


# ---------------------------------------------------------------------------
# VU Meter tool
# ---------------------------------------------------------------------------

@mcp.tool()
def set_vu_meter(ctx: Context, channel: int, value: int) -> str:
    """Set a VU meter level.

    Args:
        channel: Channel 1-8
        value: Level 0-255 (scaled to MCU 0-14 range internally)
    """
    dev = _dev(ctx)
    ch = channel - 1
    if not 0 <= ch <= 7:
        return json.dumps({"error": "Channel must be 1-8"})
    if not 0 <= value <= 255:
        return json.dumps({"error": "Value must be 0-255"})

    vu_level = (value * 14) // 255
    data = (ch << 4) | (vu_level & 0x0F)
    dev.send([0xD0, data])
    return json.dumps({"channel": channel, "value": value, "vu_level": vu_level})


# ---------------------------------------------------------------------------
# Status / input monitoring tool
# ---------------------------------------------------------------------------

@mcp.tool()
def get_status(ctx: Context) -> str:
    """Get device status and recent input events from the MCU surface."""
    dev = _dev(ctx)
    with dev._lock:
        recent = list(dev.input_buffer)

    return json.dumps({
        "handshake_state": dev.handshake_state,
        "port": dev.port_name,
        "faders": {str(i + 1): dev.fader_values[i] for i in range(9)},
        "recent_inputs": recent[-20:],
    }, indent=2)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
