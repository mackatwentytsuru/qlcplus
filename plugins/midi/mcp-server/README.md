# MCU Model Context Protocol (MCP) Server

This is an MCP server that wraps the Mackie Control Universal (MCU) MIDI protocol. It enables AI assistants (like Claude) to control physical MCU devices, specifically tested with the **Behringer X-Touch Extender**.

## Installation

To distribute this separately from QLC+, install it via `pip` or use `uvx`:

```bash
# Using uvx (recommended for Claude Desktop)
uvx mcu-mcp-server

# Or install globally
pip install mcu-mcp-server
```

## Running with Claude Desktop

Add the following to your `claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "qlc-mcu": {
      "command": "uvx",
      "args": ["mcu-mcp-server"]
    }
  }
}
```

## Features

- Motorized Faders control (14-bit precision)
- Set button LED states (on/off/blink)
- VPot LED ring control
- Write text to the LCD scribble strips
- Read actual device state, handshake, and recent inputs

See `mcu_protocol.py` for exact button mappings and constants.
