/*
  Q Light Controller Plus
  mackiecontrolprotocol.h

  Copyright (c) QLC+ Development Team

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef MACKIECONTROLPROTOCOL_H
#define MACKIECONTROLPROTOCOL_H

#include <QtGlobal>

/****************************************************************************
 * Mackie Control SysEx constants
 ****************************************************************************/
#define MCU_SYSEX_MANUFACTURER_1    0x00
#define MCU_SYSEX_MANUFACTURER_2    0x00
#define MCU_SYSEX_MANUFACTURER_3    0x66

#define MCU_DEVICE_ID_MCU           0x14  // Mackie Control Universal (Pro)
#define MCU_DEVICE_ID_MCU_XT        0x15  // Mackie Control XT (Pro)

#define MCU_SYSEX_CMD_QUERY         0x00
#define MCU_SYSEX_CMD_CHALLENGE     0x01
#define MCU_SYSEX_CMD_RESPONSE      0x02
#define MCU_SYSEX_CMD_CONFIRM       0x03
#define MCU_SYSEX_CMD_LCD           0x12
#define MCU_SYSEX_CMD_FW_VERSION    0x13
#define MCU_SYSEX_CMD_CH_METER      0x20
#define MCU_SYSEX_CMD_LCD_METER     0x21

/****************************************************************************
 * Mackie Control button Note numbers (MIDI channel 0)
 ****************************************************************************/
// Per-channel buttons (8 channels, note = base + channel 0-7)
#define MCU_NOTE_REC_BASE           0x00  // 0x00-0x07: REC/Ready 1-8
#define MCU_NOTE_SOLO_BASE          0x08  // 0x08-0x0F: Solo 1-8
#define MCU_NOTE_MUTE_BASE          0x10  // 0x10-0x17: Mute 1-8
#define MCU_NOTE_SELECT_BASE        0x18  // 0x18-0x1F: Select 1-8
#define MCU_NOTE_VPOT_SW_BASE       0x20  // 0x20-0x27: VPot switch/push 1-8

// Assignment buttons
#define MCU_NOTE_ASSIGN_TRACK       0x28
#define MCU_NOTE_ASSIGN_SEND        0x29
#define MCU_NOTE_ASSIGN_PAN         0x2A
#define MCU_NOTE_ASSIGN_PLUGIN      0x2B
#define MCU_NOTE_ASSIGN_EQ          0x2C
#define MCU_NOTE_ASSIGN_INSTRUMENT  0x2D

// Bank/Channel navigation
#define MCU_NOTE_BANK_LEFT          0x2E
#define MCU_NOTE_BANK_RIGHT         0x2F
#define MCU_NOTE_CH_LEFT            0x30
#define MCU_NOTE_CH_RIGHT           0x31

// Flip and Global View
#define MCU_NOTE_FLIP               0x32
#define MCU_NOTE_GLOBAL_VIEW        0x33

// Display buttons
#define MCU_NOTE_NAME_VALUE         0x34
#define MCU_NOTE_SMPTE_BEATS        0x35

// Function buttons
#define MCU_NOTE_F1                 0x36
#define MCU_NOTE_F2                 0x37
#define MCU_NOTE_F3                 0x38
#define MCU_NOTE_F4                 0x39
#define MCU_NOTE_F5                 0x3A
#define MCU_NOTE_F6                 0x3B
#define MCU_NOTE_F7                 0x3C
#define MCU_NOTE_F8                 0x3D

// Global View buttons
#define MCU_NOTE_MIDI_TRACKS        0x3E
#define MCU_NOTE_INPUTS             0x3F
#define MCU_NOTE_AUDIO_TRACKS       0x40
#define MCU_NOTE_AUDIO_INST         0x41
#define MCU_NOTE_AUX                0x42
#define MCU_NOTE_BUSSES             0x43
#define MCU_NOTE_OUTPUTS            0x44
#define MCU_NOTE_USER               0x45

// Modifier keys
#define MCU_NOTE_SHIFT              0x46
#define MCU_NOTE_OPTION             0x47
#define MCU_NOTE_CONTROL            0x48
#define MCU_NOTE_ALT                0x49

// Automation
#define MCU_NOTE_READ               0x4A
#define MCU_NOTE_WRITE              0x4B
#define MCU_NOTE_TRIM               0x4C
#define MCU_NOTE_TOUCH              0x4D
#define MCU_NOTE_LATCH              0x4E
#define MCU_NOTE_GROUP              0x4F

// Utility
#define MCU_NOTE_SAVE               0x50
#define MCU_NOTE_UNDO               0x51
#define MCU_NOTE_CANCEL             0x52
#define MCU_NOTE_ENTER              0x53

// Markers
#define MCU_NOTE_MARKER             0x54
#define MCU_NOTE_NUDGE              0x55
#define MCU_NOTE_CYCLE              0x56
#define MCU_NOTE_DROP               0x57
#define MCU_NOTE_REPLACE            0x58
#define MCU_NOTE_CLICK              0x59
#define MCU_NOTE_SOLO_DEFEAT        0x5A

// Transport
#define MCU_NOTE_REWIND             0x5B
#define MCU_NOTE_FORWARD            0x5C
#define MCU_NOTE_STOP               0x5D
#define MCU_NOTE_PLAY               0x5E
#define MCU_NOTE_RECORD             0x5F

// Cursor/Zoom
#define MCU_NOTE_CURSOR_UP          0x60
#define MCU_NOTE_CURSOR_DOWN        0x61
#define MCU_NOTE_CURSOR_LEFT        0x62
#define MCU_NOTE_CURSOR_RIGHT       0x63
#define MCU_NOTE_ZOOM               0x64
#define MCU_NOTE_SCRUB              0x65

// User switches
#define MCU_NOTE_USER_A             0x66
#define MCU_NOTE_USER_B             0x67

// Fader touch (note = base + channel 0-7, + 8 for master)
#define MCU_NOTE_FADER_TOUCH_BASE   0x68  // 0x68-0x6F: Fader touch 1-8
#define MCU_NOTE_FADER_TOUCH_MASTER 0x70  // Master fader touch

/****************************************************************************
 * Mackie Control CC numbers
 ****************************************************************************/
#define MCU_CC_VPOT_BASE            0x10  // CC 16-23: VPot rotation 1-8
#define MCU_CC_VPOT_LED_BASE        0x30  // CC 48-55: VPot LED ring 1-8
#define MCU_CC_JOG_WHEEL            0x3C  // CC 60: Jog/Scrub wheel
#define MCU_CC_7SEG_BASE            0x40  // CC 64-75: 7-segment display

/****************************************************************************
 * Mackie Control QLC+ channel offsets
 ****************************************************************************/
#define MACKIE_FADER_OFFSET             0   // Ch 0-8: Faders 1-8 + master
#define MACKIE_FADER_COUNT              9
#define MACKIE_VPOT_OFFSET              9   // Ch 9-16: VPot rotation 1-8
#define MACKIE_VPOT_COUNT               8
#define MACKIE_VPOT_PUSH_OFFSET         17  // Ch 17-24: VPot push 1-8
#define MACKIE_VPOT_PUSH_COUNT          8
#define MACKIE_REC_OFFSET               25  // Ch 25-32: REC 1-8
#define MACKIE_SOLO_OFFSET              33  // Ch 33-40: Solo 1-8
#define MACKIE_MUTE_OFFSET              41  // Ch 41-48: Mute 1-8
#define MACKIE_SELECT_OFFSET            49  // Ch 49-56: Select 1-8
#define MACKIE_FADER_TOUCH_OFFSET       57  // Ch 57-65: Fader touch 1-8 + master
#define MACKIE_TRANSPORT_OFFSET         66  // Ch 66-71: REW/FF/STOP/PLAY/REC/CYCLE
#define MACKIE_FUNCTION_OFFSET          72  // Ch 72-79: F1-F8
#define MACKIE_ASSIGNMENT_OFFSET        80  // Ch 80-85: Assignment buttons
#define MACKIE_BANK_OFFSET              86  // Ch 86-89: Bank/Ch navigation
#define MACKIE_MODIFIER_OFFSET          90  // Ch 90-93: Shift/Option/Ctrl/Alt
#define MACKIE_AUTOMATION_OFFSET        94  // Ch 94-99: Read/Write/Trim/Touch/Latch/Group
#define MACKIE_UTILITY_OFFSET           100 // Ch 100-103: Save/Undo/Cancel/Enter
#define MACKIE_CURSOR_OFFSET            104 // Ch 104-109: Up/Down/Left/Right/Zoom/Scrub
#define MACKIE_MISC_OFFSET              110 // Ch 110+: Other buttons
#define MACKIE_JOG_OFFSET               124 // Ch 124: Jog wheel

// Feedback-only channels (output to controller)
#define MACKIE_VPOT_LED_OFFSET          200 // Ch 200-207: VPot LED rings
#define MACKIE_7SEG_OFFSET              208 // Ch 208-219: 7-segment digits
#define MACKIE_VU_OFFSET                220 // Ch 220-227: VU meters

/****************************************************************************
 * Mackie Control VPot LED ring modes
 ****************************************************************************/
#define MCU_VPOT_MODE_SINGLE            0x00  // Single dot
#define MCU_VPOT_MODE_BOOST_CUT         0x01  // Boost/cut (center detent)
#define MCU_VPOT_MODE_WRAP              0x02  // Wrap (fills from left)
#define MCU_VPOT_MODE_SPREAD            0x03  // Spread (from center)

/****************************************************************************
 * Mackie Control Protocol conversion functions
 ****************************************************************************/
namespace MackieControlProtocol
{
    /**
     * Convert Mackie Control MIDI message to QLC+ input channel/value
     *
     * @param cmd MIDI command byte (status byte including channel)
     * @param data1 MIDI first data byte
     * @param data2 MIDI second data byte
     * @param channel Output: QLC+ input channel number
     * @param value Output: QLC+ input value (0-255)
     * @return true if successfully parsed
     */
    bool mackieToInput(uchar cmd, uchar data1, uchar data2,
                       quint32* channel, uchar* value);

    /**
     * Convert QLC+ feedback to Mackie Control MIDI message
     *
     * @param channel QLC+ input channel number
     * @param value QLC+ feedback value (0-255)
     * @param cmd Output: MIDI command byte
     * @param data1 Output: MIDI first data byte
     * @param data2 Output: MIDI second data byte
     * @return true if successfully converted
     */
    bool feedbackToMackie(quint32 channel, uchar value,
                          uchar* cmd, uchar* data1, uchar* data2);

    /**
     * Map a Mackie Control button note number to QLC+ channel
     *
     * @param note MIDI note number
     * @return QLC+ channel number, or UINT_MAX if not mapped
     */
    quint32 noteToChannel(uchar note);

    /**
     * Map a QLC+ channel to Mackie Control button note number
     *
     * @param channel QLC+ channel number
     * @return MIDI note number, or 0xFF if not a button channel
     */
    uchar channelToNote(quint32 channel);

    /**
     * Encode a VPot LED ring value
     *
     * @param position LED ring position (0-11, 0=off)
     * @param mode Ring display mode (single/boost-cut/wrap/spread)
     * @param centerLed true to turn on the center LED
     * @return Encoded CC value for VPot LED ring
     */
    uchar encodeVPotLed(uchar position, uchar mode, bool centerLed = false);
}

#endif // MACKIECONTROLPROTOCOL_H
