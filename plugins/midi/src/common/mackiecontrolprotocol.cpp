/*
  Q Light Controller Plus
  mackiecontrolprotocol.cpp

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

#include <QtCore>
#include <climits>

#include "mackiecontrolprotocol.h"
#include "midiprotocol.h"

/****************************************************************************
 * Mackie Control → QLC+ Input conversion
 ****************************************************************************/

bool MackieControlProtocol::mackieToInput(uchar cmd, uchar data1, uchar data2,
                                           quint32* channel, uchar* value)
{
    if (!MIDI_IS_CMD(cmd))
        return false;

    uchar midiCmd = MIDI_CMD(cmd);
    uchar midiCh = MIDI_CH(cmd);

    switch (midiCmd)
    {
        case MIDI_PITCH_WHEEL:
        {
            // Faders: Pitch Bend on channels 0-8
            // data1 = LSB (7 bits), data2 = MSB (7 bits)
            // 14-bit value = (MSB << 7) | LSB, range 0-16383
            // Convert to 8-bit: use MSB shifted + top bit of LSB
            if (midiCh > 8)
                return false;
            *channel = MACKIE_FADER_OFFSET + midiCh;
            *value = (data2 << 1) | ((data1 >> 6) & 0x01);
            return true;
        }

        case MIDI_NOTE_ON:
        {
            // Buttons: Note On on channel 0
            quint32 mapped = noteToChannel(data1);
            if (mapped == UINT_MAX)
                return false;
            *channel = mapped;
            *value = (data2 > 0) ? 255 : 0;
            return true;
        }

        case MIDI_NOTE_OFF:
        {
            // Button release
            quint32 mapped = noteToChannel(data1);
            if (mapped == UINT_MAX)
                return false;
            *channel = mapped;
            *value = 0;
            return true;
        }

        case MIDI_CONTROL_CHANGE:
        {
            // VPots: CC 16-23 (relative encoders)
            if (data1 >= MCU_CC_VPOT_BASE && data1 < MCU_CC_VPOT_BASE + MACKIE_VPOT_COUNT)
            {
                *channel = MACKIE_VPOT_OFFSET + (data1 - MCU_CC_VPOT_BASE);
                // Relative encoding: CW = 0x01-0x0F, CCW = 0x41-0x4F
                // Convert to encoder-compatible: >127 = increment, <127 = decrement
                if (data2 >= 0x01 && data2 <= 0x0F)
                    *value = 127 + data2;  // 128-142 (increment)
                else if (data2 >= 0x41 && data2 <= 0x4F)
                    *value = 127 - (data2 - 0x40);  // 112-126 (decrement)
                else
                    *value = 127;  // no change
                return true;
            }
            // Jog wheel: CC 60
            else if (data1 == MCU_CC_JOG_WHEEL)
            {
                *channel = MACKIE_JOG_OFFSET;
                if (data2 >= 0x01 && data2 <= 0x0F)
                    *value = 127 + data2;
                else if (data2 >= 0x41 && data2 <= 0x4F)
                    *value = 127 - (data2 - 0x40);
                else
                    *value = 127;
                return true;
            }
            return false;
        }

        case MIDI_CHANNEL_AFTERTOUCH:
        {
            // VU Meters: Channel Pressure
            // data1 encodes both channel (high nibble) and level (low nibble)
            uchar vuChannel = (data1 >> 4) & 0x07;
            uchar vuLevel = data1 & 0x0F;
            *channel = MACKIE_VU_OFFSET + vuChannel;
            // Scale 0-14 to 0-255 (0x0F = clear overload → 0)
            if (vuLevel == 0x0F)
            {
                *value = 0;
                return true;
            }
            if (vuLevel > 14) vuLevel = 14;
            *value = (vuLevel * 255) / 14;
            return true;
        }

        default:
            return false;
    }
}

/****************************************************************************
 * QLC+ Feedback → Mackie Control conversion
 ****************************************************************************/

bool MackieControlProtocol::feedbackToMackie(quint32 channel, uchar value,
                                              uchar* cmd, uchar* data1, uchar* data2)
{
    // Faders: channels 0-8 → Pitch Bend on MIDI channel 0-8
    if (channel < MACKIE_FADER_OFFSET + MACKIE_FADER_COUNT)
    {
        uchar midiCh = (uchar)(channel - MACKIE_FADER_OFFSET);
        *cmd = MIDI_PITCH_WHEEL | midiCh;
        // Convert 8-bit value to 14-bit pitch bend
        // value (0-255) → 14-bit (0-16383)
        quint16 faderValue = (quint32)value * 16383 / 255;
        *data1 = faderValue & 0x7F;          // LSB
        *data2 = (faderValue >> 7) & 0x7F;   // MSB
        return true;
    }

    // VPot LED rings: channels 200-207 → CC 48-55
    if (channel >= MACKIE_VPOT_LED_OFFSET &&
        channel < MACKIE_VPOT_LED_OFFSET + MACKIE_VPOT_COUNT)
    {
        *cmd = MIDI_CONTROL_CHANGE;  // Channel 0
        *data1 = MCU_CC_VPOT_LED_BASE + (channel - MACKIE_VPOT_LED_OFFSET);
        // Scale 0-255 to LED ring position 0-11 with single dot mode
        uchar position = (value * 11) / 255;
        *data2 = encodeVPotLed(position, MCU_VPOT_MODE_SINGLE);
        return true;
    }

    // 7-segment display: channels 208-219 → CC 64-75
    if (channel >= MACKIE_7SEG_OFFSET && channel < MACKIE_7SEG_OFFSET + 12)
    {
        *cmd = MIDI_CONTROL_CHANGE;  // Channel 0
        *data1 = MCU_CC_7SEG_BASE + (channel - MACKIE_7SEG_OFFSET);
        *data2 = value & 0x7F;
        return true;
    }

    // VU meters: channels 220-227 → Channel Pressure
    if (channel >= MACKIE_VU_OFFSET && channel < MACKIE_VU_OFFSET + 8)
    {
        uchar vuCh = (uchar)(channel - MACKIE_VU_OFFSET);
        *cmd = MIDI_CHANNEL_AFTERTOUCH;
        // Encode: high nibble = channel, low nibble = level (0-14)
        uchar vuLevel = (value * 14) / 255;
        *data1 = (vuCh << 4) | (vuLevel & 0x0F);
        *data2 = 0;
        return true;
    }

    // Buttons: map QLC+ channel back to note number
    uchar note = channelToNote(channel);
    if (note != 0xFF)
    {
        *cmd = MIDI_NOTE_ON;  // Always on channel 0
        *data1 = note;
        *data2 = (value > 0) ? 0x7F : 0x00;
        return true;
    }

    return false;
}

/****************************************************************************
 * Note ↔ Channel mapping
 ****************************************************************************/

quint32 MackieControlProtocol::noteToChannel(uchar note)
{
    // Per-channel buttons (8 per group)
    // MCU_NOTE_REC_BASE is 0x00, so just check the upper bound
    if (note < MCU_NOTE_REC_BASE + 8)
        return MACKIE_REC_OFFSET + (note - MCU_NOTE_REC_BASE);

    if (note >= MCU_NOTE_SOLO_BASE && note < MCU_NOTE_SOLO_BASE + 8)
        return MACKIE_SOLO_OFFSET + (note - MCU_NOTE_SOLO_BASE);

    if (note >= MCU_NOTE_MUTE_BASE && note < MCU_NOTE_MUTE_BASE + 8)
        return MACKIE_MUTE_OFFSET + (note - MCU_NOTE_MUTE_BASE);

    if (note >= MCU_NOTE_SELECT_BASE && note < MCU_NOTE_SELECT_BASE + 8)
        return MACKIE_SELECT_OFFSET + (note - MCU_NOTE_SELECT_BASE);

    if (note >= MCU_NOTE_VPOT_SW_BASE && note < MCU_NOTE_VPOT_SW_BASE + 8)
        return MACKIE_VPOT_PUSH_OFFSET + (note - MCU_NOTE_VPOT_SW_BASE);

    // Fader touch
    if (note >= MCU_NOTE_FADER_TOUCH_BASE && note <= MCU_NOTE_FADER_TOUCH_MASTER)
        return MACKIE_FADER_TOUCH_OFFSET + (note - MCU_NOTE_FADER_TOUCH_BASE);

    // Transport buttons (contiguous block 0x5B-0x5F + 0x56 for Cycle)
    switch (note)
    {
        case MCU_NOTE_REWIND:   return MACKIE_TRANSPORT_OFFSET + 0;
        case MCU_NOTE_FORWARD:  return MACKIE_TRANSPORT_OFFSET + 1;
        case MCU_NOTE_STOP:     return MACKIE_TRANSPORT_OFFSET + 2;
        case MCU_NOTE_PLAY:     return MACKIE_TRANSPORT_OFFSET + 3;
        case MCU_NOTE_RECORD:   return MACKIE_TRANSPORT_OFFSET + 4;
        case MCU_NOTE_CYCLE:    return MACKIE_TRANSPORT_OFFSET + 5;
    }

    // Function buttons F1-F8
    if (note >= MCU_NOTE_F1 && note <= MCU_NOTE_F8)
        return MACKIE_FUNCTION_OFFSET + (note - MCU_NOTE_F1);

    // Assignment buttons
    switch (note)
    {
        case MCU_NOTE_ASSIGN_TRACK:      return MACKIE_ASSIGNMENT_OFFSET + 0;
        case MCU_NOTE_ASSIGN_SEND:       return MACKIE_ASSIGNMENT_OFFSET + 1;
        case MCU_NOTE_ASSIGN_PAN:        return MACKIE_ASSIGNMENT_OFFSET + 2;
        case MCU_NOTE_ASSIGN_PLUGIN:     return MACKIE_ASSIGNMENT_OFFSET + 3;
        case MCU_NOTE_ASSIGN_EQ:         return MACKIE_ASSIGNMENT_OFFSET + 4;
        case MCU_NOTE_ASSIGN_INSTRUMENT: return MACKIE_ASSIGNMENT_OFFSET + 5;
    }

    // Bank/Channel navigation
    switch (note)
    {
        case MCU_NOTE_BANK_LEFT:   return MACKIE_BANK_OFFSET + 0;
        case MCU_NOTE_BANK_RIGHT:  return MACKIE_BANK_OFFSET + 1;
        case MCU_NOTE_CH_LEFT:     return MACKIE_BANK_OFFSET + 2;
        case MCU_NOTE_CH_RIGHT:    return MACKIE_BANK_OFFSET + 3;
    }

    // Modifier keys
    switch (note)
    {
        case MCU_NOTE_SHIFT:   return MACKIE_MODIFIER_OFFSET + 0;
        case MCU_NOTE_OPTION:  return MACKIE_MODIFIER_OFFSET + 1;
        case MCU_NOTE_CONTROL: return MACKIE_MODIFIER_OFFSET + 2;
        case MCU_NOTE_ALT:     return MACKIE_MODIFIER_OFFSET + 3;
    }

    // Automation buttons
    switch (note)
    {
        case MCU_NOTE_READ:    return MACKIE_AUTOMATION_OFFSET + 0;
        case MCU_NOTE_WRITE:   return MACKIE_AUTOMATION_OFFSET + 1;
        case MCU_NOTE_TRIM:    return MACKIE_AUTOMATION_OFFSET + 2;
        case MCU_NOTE_TOUCH:   return MACKIE_AUTOMATION_OFFSET + 3;
        case MCU_NOTE_LATCH:   return MACKIE_AUTOMATION_OFFSET + 4;
        case MCU_NOTE_GROUP:   return MACKIE_AUTOMATION_OFFSET + 5;
    }

    // Utility buttons
    switch (note)
    {
        case MCU_NOTE_SAVE:    return MACKIE_UTILITY_OFFSET + 0;
        case MCU_NOTE_UNDO:    return MACKIE_UTILITY_OFFSET + 1;
        case MCU_NOTE_CANCEL:  return MACKIE_UTILITY_OFFSET + 2;
        case MCU_NOTE_ENTER:   return MACKIE_UTILITY_OFFSET + 3;
    }

    // Cursor/Zoom buttons
    switch (note)
    {
        case MCU_NOTE_CURSOR_UP:    return MACKIE_CURSOR_OFFSET + 0;
        case MCU_NOTE_CURSOR_DOWN:  return MACKIE_CURSOR_OFFSET + 1;
        case MCU_NOTE_CURSOR_LEFT:  return MACKIE_CURSOR_OFFSET + 2;
        case MCU_NOTE_CURSOR_RIGHT: return MACKIE_CURSOR_OFFSET + 3;
        case MCU_NOTE_ZOOM:         return MACKIE_CURSOR_OFFSET + 4;
        case MCU_NOTE_SCRUB:        return MACKIE_CURSOR_OFFSET + 5;
    }

    // Misc buttons
    switch (note)
    {
        case MCU_NOTE_FLIP:         return MACKIE_MISC_OFFSET + 0;
        case MCU_NOTE_GLOBAL_VIEW:  return MACKIE_MISC_OFFSET + 1;
        case MCU_NOTE_NAME_VALUE:   return MACKIE_MISC_OFFSET + 2;
        case MCU_NOTE_SMPTE_BEATS:  return MACKIE_MISC_OFFSET + 3;
        case MCU_NOTE_MARKER:       return MACKIE_MISC_OFFSET + 4;
        case MCU_NOTE_NUDGE:        return MACKIE_MISC_OFFSET + 5;
        case MCU_NOTE_DROP:         return MACKIE_MISC_OFFSET + 6;
        case MCU_NOTE_REPLACE:      return MACKIE_MISC_OFFSET + 7;
        case MCU_NOTE_CLICK:        return MACKIE_MISC_OFFSET + 8;
        case MCU_NOTE_SOLO_DEFEAT:  return MACKIE_MISC_OFFSET + 9;
        case MCU_NOTE_USER_A:       return MACKIE_MISC_OFFSET + 10;
        case MCU_NOTE_USER_B:       return MACKIE_MISC_OFFSET + 11;
        // Global View sub-buttons
        case MCU_NOTE_MIDI_TRACKS:   return MACKIE_MISC_OFFSET + 12;
        case MCU_NOTE_INPUTS:        return MACKIE_MISC_OFFSET + 13;
        case MCU_NOTE_AUDIO_TRACKS:  return MACKIE_MISC_OFFSET + 14;
        case MCU_NOTE_AUDIO_INST:    return MACKIE_MISC_OFFSET + 15;
        case MCU_NOTE_AUX:           return MACKIE_MISC_OFFSET + 16;
        case MCU_NOTE_BUSSES:        return MACKIE_MISC_OFFSET + 17;
        case MCU_NOTE_OUTPUTS:       return MACKIE_MISC_OFFSET + 18;
        case MCU_NOTE_USER:          return MACKIE_MISC_OFFSET + 19;
    }

    return UINT_MAX;  // Unknown note
}

uchar MackieControlProtocol::channelToNote(quint32 channel)
{
    // Per-channel button groups
    if (channel >= MACKIE_REC_OFFSET && channel < MACKIE_REC_OFFSET + 8)
        return MCU_NOTE_REC_BASE + (channel - MACKIE_REC_OFFSET);

    if (channel >= MACKIE_SOLO_OFFSET && channel < MACKIE_SOLO_OFFSET + 8)
        return MCU_NOTE_SOLO_BASE + (channel - MACKIE_SOLO_OFFSET);

    if (channel >= MACKIE_MUTE_OFFSET && channel < MACKIE_MUTE_OFFSET + 8)
        return MCU_NOTE_MUTE_BASE + (channel - MACKIE_MUTE_OFFSET);

    if (channel >= MACKIE_SELECT_OFFSET && channel < MACKIE_SELECT_OFFSET + 8)
        return MCU_NOTE_SELECT_BASE + (channel - MACKIE_SELECT_OFFSET);

    if (channel >= MACKIE_VPOT_PUSH_OFFSET && channel < MACKIE_VPOT_PUSH_OFFSET + 8)
        return MCU_NOTE_VPOT_SW_BASE + (channel - MACKIE_VPOT_PUSH_OFFSET);

    // Fader touch
    if (channel >= MACKIE_FADER_TOUCH_OFFSET && channel < MACKIE_FADER_TOUCH_OFFSET + 9)
        return MCU_NOTE_FADER_TOUCH_BASE + (channel - MACKIE_FADER_TOUCH_OFFSET);

    // Transport
    static const uchar transportNotes[] = {
        MCU_NOTE_REWIND, MCU_NOTE_FORWARD, MCU_NOTE_STOP,
        MCU_NOTE_PLAY, MCU_NOTE_RECORD, MCU_NOTE_CYCLE
    };
    if (channel >= MACKIE_TRANSPORT_OFFSET && channel < MACKIE_TRANSPORT_OFFSET + 6)
        return transportNotes[channel - MACKIE_TRANSPORT_OFFSET];

    // Function buttons
    if (channel >= MACKIE_FUNCTION_OFFSET && channel < MACKIE_FUNCTION_OFFSET + 8)
        return MCU_NOTE_F1 + (channel - MACKIE_FUNCTION_OFFSET);

    // Assignment buttons
    static const uchar assignNotes[] = {
        MCU_NOTE_ASSIGN_TRACK, MCU_NOTE_ASSIGN_SEND, MCU_NOTE_ASSIGN_PAN,
        MCU_NOTE_ASSIGN_PLUGIN, MCU_NOTE_ASSIGN_EQ, MCU_NOTE_ASSIGN_INSTRUMENT
    };
    if (channel >= MACKIE_ASSIGNMENT_OFFSET && channel < MACKIE_ASSIGNMENT_OFFSET + 6)
        return assignNotes[channel - MACKIE_ASSIGNMENT_OFFSET];

    // Bank/Channel navigation
    static const uchar bankNotes[] = {
        MCU_NOTE_BANK_LEFT, MCU_NOTE_BANK_RIGHT,
        MCU_NOTE_CH_LEFT, MCU_NOTE_CH_RIGHT
    };
    if (channel >= MACKIE_BANK_OFFSET && channel < MACKIE_BANK_OFFSET + 4)
        return bankNotes[channel - MACKIE_BANK_OFFSET];

    // Modifier keys
    static const uchar modNotes[] = {
        MCU_NOTE_SHIFT, MCU_NOTE_OPTION, MCU_NOTE_CONTROL, MCU_NOTE_ALT
    };
    if (channel >= MACKIE_MODIFIER_OFFSET && channel < MACKIE_MODIFIER_OFFSET + 4)
        return modNotes[channel - MACKIE_MODIFIER_OFFSET];

    // Automation
    static const uchar autoNotes[] = {
        MCU_NOTE_READ, MCU_NOTE_WRITE, MCU_NOTE_TRIM,
        MCU_NOTE_TOUCH, MCU_NOTE_LATCH, MCU_NOTE_GROUP
    };
    if (channel >= MACKIE_AUTOMATION_OFFSET && channel < MACKIE_AUTOMATION_OFFSET + 6)
        return autoNotes[channel - MACKIE_AUTOMATION_OFFSET];

    // Utility
    static const uchar utilNotes[] = {
        MCU_NOTE_SAVE, MCU_NOTE_UNDO, MCU_NOTE_CANCEL, MCU_NOTE_ENTER
    };
    if (channel >= MACKIE_UTILITY_OFFSET && channel < MACKIE_UTILITY_OFFSET + 4)
        return utilNotes[channel - MACKIE_UTILITY_OFFSET];

    // Cursor/Zoom
    static const uchar cursorNotes[] = {
        MCU_NOTE_CURSOR_UP, MCU_NOTE_CURSOR_DOWN,
        MCU_NOTE_CURSOR_LEFT, MCU_NOTE_CURSOR_RIGHT,
        MCU_NOTE_ZOOM, MCU_NOTE_SCRUB
    };
    if (channel >= MACKIE_CURSOR_OFFSET && channel < MACKIE_CURSOR_OFFSET + 6)
        return cursorNotes[channel - MACKIE_CURSOR_OFFSET];

    // Misc buttons
    static const uchar miscNotes[] = {
        MCU_NOTE_FLIP, MCU_NOTE_GLOBAL_VIEW,
        MCU_NOTE_NAME_VALUE, MCU_NOTE_SMPTE_BEATS,
        MCU_NOTE_MARKER, MCU_NOTE_NUDGE,
        MCU_NOTE_DROP, MCU_NOTE_REPLACE,
        MCU_NOTE_CLICK, MCU_NOTE_SOLO_DEFEAT,
        MCU_NOTE_USER_A, MCU_NOTE_USER_B,
        MCU_NOTE_MIDI_TRACKS, MCU_NOTE_INPUTS,
        MCU_NOTE_AUDIO_TRACKS, MCU_NOTE_AUDIO_INST,
        MCU_NOTE_AUX, MCU_NOTE_BUSSES,
        MCU_NOTE_OUTPUTS, MCU_NOTE_USER
    };
    if (channel >= MACKIE_MISC_OFFSET && channel < MACKIE_MISC_OFFSET + 20)
        return miscNotes[channel - MACKIE_MISC_OFFSET];

    return 0xFF;  // Not a button channel
}

/****************************************************************************
 * VPot LED ring encoding
 ****************************************************************************/

uchar MackieControlProtocol::encodeVPotLed(uchar position, uchar mode, bool centerLed)
{
    // Bit layout: 0bCMMPPPP
    // C = center LED, MM = mode (2 bits), PPPP = position (4 bits)
    uchar encoded = (position & 0x0F);
    encoded |= ((mode & 0x03) << 4);
    if (centerLed)
        encoded |= 0x40;
    return encoded;
}
