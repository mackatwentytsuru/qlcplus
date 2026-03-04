"""
Mackie Control Universal (MCU) protocol constants and conversion functions.

Ported from QLC+ C++ implementation:
  - plugins/midi/src/common/mackiecontrolprotocol.h
  - plugins/midi/src/common/mackiecontrolhandler.cpp
"""

# ---------------------------------------------------------------------------
# SysEx constants
# ---------------------------------------------------------------------------
SYSEX_HEADER = [0x00, 0x00, 0x66]  # Mackie manufacturer ID

DEVICE_ID_MCU = 0x14     # Mackie Control Universal (Pro)
DEVICE_ID_MCU_XT = 0x15  # Mackie Control XT (Pro)

SYSEX_CMD_QUERY = 0x00
SYSEX_CMD_CHALLENGE = 0x01
SYSEX_CMD_RESPONSE = 0x02
SYSEX_CMD_CONFIRM = 0x03
SYSEX_CMD_LCD = 0x12

# ---------------------------------------------------------------------------
# Button note numbers (MIDI Note On/Off, channel 0)
# ---------------------------------------------------------------------------
# Per-channel button groups (base + channel 0-7)
NOTE_REC_BASE = 0x00
NOTE_SOLO_BASE = 0x08
NOTE_MUTE_BASE = 0x10
NOTE_SELECT_BASE = 0x18
NOTE_VPOT_SW_BASE = 0x20

# Assignment buttons
NOTE_ASSIGN_TRACK = 0x28
NOTE_ASSIGN_SEND = 0x29
NOTE_ASSIGN_PAN = 0x2A
NOTE_ASSIGN_PLUGIN = 0x2B
NOTE_ASSIGN_EQ = 0x2C
NOTE_ASSIGN_INSTRUMENT = 0x2D

# Bank/Channel navigation
NOTE_BANK_LEFT = 0x2E
NOTE_BANK_RIGHT = 0x2F
NOTE_CH_LEFT = 0x30
NOTE_CH_RIGHT = 0x31

# Flip / Global View / Display
NOTE_FLIP = 0x32
NOTE_GLOBAL_VIEW = 0x33
NOTE_NAME_VALUE = 0x34
NOTE_SMPTE_BEATS = 0x35

# Function buttons F1-F8
NOTE_F1 = 0x36
NOTE_F2 = 0x37
NOTE_F3 = 0x38
NOTE_F4 = 0x39
NOTE_F5 = 0x3A
NOTE_F6 = 0x3B
NOTE_F7 = 0x3C
NOTE_F8 = 0x3D

# Global View buttons
NOTE_MIDI_TRACKS = 0x3E
NOTE_INPUTS = 0x3F
NOTE_AUDIO_TRACKS = 0x40
NOTE_AUDIO_INST = 0x41
NOTE_AUX = 0x42
NOTE_BUSSES = 0x43
NOTE_OUTPUTS = 0x44
NOTE_USER = 0x45

# Modifier keys
NOTE_SHIFT = 0x46
NOTE_OPTION = 0x47
NOTE_CONTROL = 0x48
NOTE_ALT = 0x49

# Automation
NOTE_READ = 0x4A
NOTE_WRITE = 0x4B
NOTE_TRIM = 0x4C
NOTE_TOUCH = 0x4D
NOTE_LATCH = 0x4E
NOTE_GROUP = 0x4F

# Utility
NOTE_SAVE = 0x50
NOTE_UNDO = 0x51
NOTE_CANCEL = 0x52
NOTE_ENTER = 0x53

# Markers
NOTE_MARKER = 0x54
NOTE_NUDGE = 0x55
NOTE_CYCLE = 0x56
NOTE_DROP = 0x57
NOTE_REPLACE = 0x58
NOTE_CLICK = 0x59
NOTE_SOLO_DEFEAT = 0x5A

# Transport
NOTE_REWIND = 0x5B
NOTE_FORWARD = 0x5C
NOTE_STOP = 0x5D
NOTE_PLAY = 0x5E
NOTE_RECORD = 0x5F

# Cursor/Zoom
NOTE_CURSOR_UP = 0x60
NOTE_CURSOR_DOWN = 0x61
NOTE_CURSOR_LEFT = 0x62
NOTE_CURSOR_RIGHT = 0x63
NOTE_ZOOM = 0x64
NOTE_SCRUB = 0x65

# User switches
NOTE_USER_A = 0x66
NOTE_USER_B = 0x67

# Fader touch
NOTE_FADER_TOUCH_BASE = 0x68   # 0x68-0x6F: ch 1-8
NOTE_FADER_TOUCH_MASTER = 0x70

# ---------------------------------------------------------------------------
# CC numbers
# ---------------------------------------------------------------------------
CC_VPOT_BASE = 0x10      # CC 16-23: VPot rotation 1-8
CC_VPOT_LED_BASE = 0x30  # CC 48-55: VPot LED ring 1-8
CC_JOG_WHEEL = 0x3C      # CC 60: Jog/Scrub wheel
CC_7SEG_BASE = 0x40      # CC 64-75: 7-segment display

# ---------------------------------------------------------------------------
# VPot LED ring modes
# ---------------------------------------------------------------------------
VPOT_MODE_SINGLE = 0     # Single dot
VPOT_MODE_BOOST_CUT = 1  # Boost/cut (center detent)
VPOT_MODE_WRAP = 2       # Wrap (fills from left)
VPOT_MODE_SPREAD = 3     # Spread (from center)

# ---------------------------------------------------------------------------
# Human-readable button name → MIDI note mapping (for AI-friendly API)
# ---------------------------------------------------------------------------
BUTTON_NAMES: dict[str, int] = {
    # Per-channel buttons (1-indexed for human readability)
    **{f"rec{i+1}": NOTE_REC_BASE + i for i in range(8)},
    **{f"solo{i+1}": NOTE_SOLO_BASE + i for i in range(8)},
    **{f"mute{i+1}": NOTE_MUTE_BASE + i for i in range(8)},
    **{f"select{i+1}": NOTE_SELECT_BASE + i for i in range(8)},
    **{f"vpot_push{i+1}": NOTE_VPOT_SW_BASE + i for i in range(8)},
    **{f"fader_touch{i+1}": NOTE_FADER_TOUCH_BASE + i for i in range(8)},
    "fader_touch_master": NOTE_FADER_TOUCH_MASTER,
    # Assignment
    "track": NOTE_ASSIGN_TRACK, "send": NOTE_ASSIGN_SEND,
    "pan": NOTE_ASSIGN_PAN, "plugin": NOTE_ASSIGN_PLUGIN,
    "eq": NOTE_ASSIGN_EQ, "instrument": NOTE_ASSIGN_INSTRUMENT,
    # Bank/Channel
    "bank_left": NOTE_BANK_LEFT, "bank_right": NOTE_BANK_RIGHT,
    "ch_left": NOTE_CH_LEFT, "ch_right": NOTE_CH_RIGHT,
    # Display
    "flip": NOTE_FLIP, "global_view": NOTE_GLOBAL_VIEW,
    "name_value": NOTE_NAME_VALUE, "smpte_beats": NOTE_SMPTE_BEATS,
    # Function buttons
    **{f"f{i+1}": NOTE_F1 + i for i in range(8)},
    # Global View
    "midi_tracks": NOTE_MIDI_TRACKS, "inputs": NOTE_INPUTS,
    "audio_tracks": NOTE_AUDIO_TRACKS, "audio_inst": NOTE_AUDIO_INST,
    "aux": NOTE_AUX, "busses": NOTE_BUSSES,
    "outputs": NOTE_OUTPUTS, "user": NOTE_USER,
    # Modifiers
    "shift": NOTE_SHIFT, "option": NOTE_OPTION,
    "control": NOTE_CONTROL, "alt": NOTE_ALT,
    # Automation
    "read": NOTE_READ, "write": NOTE_WRITE, "trim": NOTE_TRIM,
    "touch": NOTE_TOUCH, "latch": NOTE_LATCH, "group": NOTE_GROUP,
    # Utility
    "save": NOTE_SAVE, "undo": NOTE_UNDO,
    "cancel": NOTE_CANCEL, "enter": NOTE_ENTER,
    # Markers
    "marker": NOTE_MARKER, "nudge": NOTE_NUDGE,
    "cycle": NOTE_CYCLE, "drop": NOTE_DROP,
    "replace": NOTE_REPLACE, "click": NOTE_CLICK,
    "solo_defeat": NOTE_SOLO_DEFEAT,
    # Transport
    "rewind": NOTE_REWIND, "forward": NOTE_FORWARD,
    "stop": NOTE_STOP, "play": NOTE_PLAY, "record": NOTE_RECORD,
    # Cursor
    "up": NOTE_CURSOR_UP, "down": NOTE_CURSOR_DOWN,
    "left": NOTE_CURSOR_LEFT, "right": NOTE_CURSOR_RIGHT,
    "zoom": NOTE_ZOOM, "scrub": NOTE_SCRUB,
    # User switches
    "user_a": NOTE_USER_A, "user_b": NOTE_USER_B,
}

# Reverse mapping: note number → button name
NOTE_TO_NAME: dict[int, str] = {v: k for k, v in BUTTON_NAMES.items()}

# ---------------------------------------------------------------------------
# Conversion functions
# ---------------------------------------------------------------------------

def value_to_pitch_bend(value: int) -> tuple[int, int]:
    """Convert 8-bit value (0-255) to 14-bit pitch bend (LSB, MSB).

    Uses the same formula as QLC+ mackiecontrolprotocol.cpp:
        faderValue = value * 16383 / 255
    """
    pb = value * 16383 // 255
    return (pb & 0x7F, (pb >> 7) & 0x7F)


def pitch_bend_to_value(lsb: int, msb: int) -> int:
    """Convert 14-bit pitch bend to 8-bit value (0-255).

    Uses the same formula as QLC+ mackiecontrolprotocol.cpp:
        value = (data2 << 1) | ((data1 >> 6) & 0x01)
    """
    return (msb << 1) | ((lsb >> 6) & 0x01)


def encode_vpot_led(position: int, mode: int = VPOT_MODE_SINGLE,
                    center_led: bool = False) -> int:
    """Encode VPot LED ring value for CC 48-55.

    Args:
        position: LED position 0-11 (0=off, 1-11 = ring positions)
        mode: Display mode (VPOT_MODE_SINGLE/BOOST_CUT/WRAP/SPREAD)
        center_led: Whether to light the center LED

    Returns:
        Encoded CC value byte
    """
    return ((position & 0x0F)
            | ((mode & 0x03) << 4)
            | (0x40 if center_led else 0))


def compute_challenge_response(challenge: list[int]) -> list[int]:
    """Compute MCU handshake challenge-response.

    Port of MackieControlHandler::computeChallengeResponse() from
    mackiecontrolhandler.cpp.

    Args:
        challenge: 4-byte challenge from device

    Returns:
        4-byte response
    """
    c = challenge
    r = [0] * 4
    r[0] = 0x7F & (c[0] + (c[1] ^ 0x0A) - c[3])
    r[1] = 0x7F & ((c[2] >> 4) ^ (c[0] + c[3]))
    r[2] = 0x7F & ((c[3] - (c[2] << 2)) ^ (c[0] | c[1]))
    r[3] = 0x7F & (c[1] - c[2] + (0xF0 ^ (c[3] << 4)))
    return r


def build_sysex(device_id: int, command: int, data: list[int]) -> list[int]:
    """Build a complete MCU SysEx message.

    Returns:
        Complete SysEx message including F0 header and F7 terminator
    """
    return [0xF0] + SYSEX_HEADER + [device_id, command] + data + [0xF7]


def decode_midi_input(message: list[int]) -> dict | None:
    """Decode an incoming MIDI message from an MCU device.

    Returns a dict describing the event, or None if not recognized.
    """
    if len(message) < 1:
        return None

    status = message[0]
    cmd = status & 0xF0
    midi_ch = status & 0x0F

    # Pitch Bend → fader position
    if cmd == 0xE0 and len(message) >= 3 and midi_ch <= 8:
        value = pitch_bend_to_value(message[1], message[2])
        return {
            "type": "fader",
            "channel": midi_ch,
            "value": value,
        }

    # Note On → button press/release
    if cmd == 0x90 and len(message) >= 3:
        note = message[1]
        name = NOTE_TO_NAME.get(note, f"note_{note:#04x}")
        return {
            "type": "button",
            "name": name,
            "note": note,
            "pressed": message[2] > 0,
        }

    # Note Off → button release
    if cmd == 0x80 and len(message) >= 3:
        note = message[1]
        name = NOTE_TO_NAME.get(note, f"note_{note:#04x}")
        return {
            "type": "button",
            "name": name,
            "note": note,
            "pressed": False,
        }

    # Control Change → VPot rotation, Jog wheel
    if cmd == 0xB0 and len(message) >= 3:
        cc = message[1]
        data2 = message[2]

        # VPot rotation (CC 16-23)
        if CC_VPOT_BASE <= cc <= CC_VPOT_BASE + 7:
            vpot = cc - CC_VPOT_BASE
            if data2 < 0x40:
                direction = "clockwise"
                speed = data2
            else:
                direction = "counter_clockwise"
                speed = data2 - 0x40
            return {
                "type": "vpot",
                "channel": vpot,
                "direction": direction,
                "speed": speed,
            }

        # Jog wheel (CC 60)
        if cc == CC_JOG_WHEEL:
            if data2 < 0x40:
                direction = "clockwise"
                speed = data2
            else:
                direction = "counter_clockwise"
                speed = data2 - 0x40
            return {
                "type": "jog",
                "direction": direction,
                "speed": speed,
            }

    # Channel Pressure → VU meter (from device, uncommon but possible)
    if cmd == 0xD0 and len(message) >= 2:
        vu_ch = (message[1] >> 4) & 0x07
        vu_level = message[1] & 0x0F
        return {
            "type": "vu_meter",
            "channel": vu_ch,
            "level": vu_level,
        }

    # SysEx
    if status == 0xF0:
        return {
            "type": "sysex",
            "data": message,
        }

    return None
