/*
  Q Light Controller Plus
  midi_test.cpp

  Copyright (c) Jano Svitok

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

#include <QTest>
#include <climits>

#define private public
#include "midi_test.h"
#include "midiprotocol.h"
#include "mackiecontrolprotocol.h"

#undef private

/****************************************************************************
 * Original MIDI protocol tests
 ****************************************************************************/

void Midi_Test::midiToInput()
{
    quint32 channel = 0;
    uchar value = 0;

    uchar midiChannel = 7;
    uchar cmd = MIDI_NOTE_ON | midiChannel;
    uchar data1 = 10;
    uchar data2 = 127;

    QLCMIDIProtocol::midiToInput(cmd, data1, data2, midiChannel, &channel, &value);

    QCOMPARE(channel, 138U);
    QCOMPARE(value, uchar(255U));
}

/****************************************************************************
 * Mackie Control Protocol: Input conversion tests
 ****************************************************************************/

void Midi_Test::mackieToInput_faders()
{
    quint32 channel = 0;
    uchar value = 0;

    // Fader 1 at mid position: Pitch Bend ch 0, data1=0x00, data2=0x40
    // value = (0x40 << 1) | ((0x00 >> 6) & 0x01) = 128
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_PITCH_WHEEL | 0, 0x00, 0x40, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_FADER_OFFSET + 0));
    QCOMPARE(value, uchar(128));

    // Fader 1 at zero: data1=0, data2=0
    // value = (0 << 1) | 0 = 0
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_PITCH_WHEEL | 0, 0x00, 0x00, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_FADER_OFFSET + 0));
    QCOMPARE(value, uchar(0));

    // Fader 1 at maximum: data1=0x7F, data2=0x7F
    // value = (0x7F << 1) | ((0x7F >> 6) & 0x01) = 254 | 1 = 255
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_PITCH_WHEEL | 0, 0x7F, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_FADER_OFFSET + 0));
    QCOMPARE(value, uchar(255));

    // Master fader (ch 8) at mid position
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_PITCH_WHEEL | 8, 0x00, 0x40, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_FADER_OFFSET + 8));
    QCOMPARE(value, uchar(128));

    // Out of range: Pitch Bend ch 9 should fail
    QVERIFY(!MackieControlProtocol::mackieToInput(
        MIDI_PITCH_WHEEL | 9, 0x00, 0x40, &channel, &value));

    // Fader 5 (ch 4) with LSB=0x40 (bit 6 set)
    // value = (0x40 << 1) | ((0x40 >> 6) & 0x01) = 128 | 1 = 129
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_PITCH_WHEEL | 4, 0x40, 0x40, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_FADER_OFFSET + 4));
    QCOMPARE(value, uchar(129));
}

void Midi_Test::mackieToInput_buttons()
{
    quint32 channel = 0;
    uchar value = 0;

    // REC 1 pressed: Note On note=0x00, vel=0x7F
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_REC_BASE, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_REC_OFFSET);
    QCOMPARE(value, uchar(255));

    // REC 1 released via vel=0x00: Note On with velocity 0
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_REC_BASE, 0x00, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_REC_OFFSET);
    QCOMPARE(value, uchar(0));

    // SOLO 1 released: Note Off note=0x08
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_OFF, MCU_NOTE_SOLO_BASE, 0x00, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_SOLO_OFFSET);
    QCOMPARE(value, uchar(0));

    // PLAY pressed: Note On note=0x5E, vel=0x7F
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_PLAY, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_TRANSPORT_OFFSET + 3));
    QCOMPARE(value, uchar(255));

    // MUTE 8 pressed: Note On note=0x17, vel=0x7F
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_MUTE_BASE + 7, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_MUTE_OFFSET + 7));
    QCOMPARE(value, uchar(255));

    // SELECT 1 pressed
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_SELECT_BASE, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_SELECT_OFFSET);
    QCOMPARE(value, uchar(255));

    // VPot push 1
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_VPOT_SW_BASE, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_VPOT_PUSH_OFFSET);
    QCOMPARE(value, uchar(255));

    // Fader touch 1
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_FADER_TOUCH_BASE, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_FADER_TOUCH_OFFSET);
    QCOMPARE(value, uchar(255));

    // Master fader touch
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_FADER_TOUCH_MASTER, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_FADER_TOUCH_OFFSET + 8));
    QCOMPARE(value, uchar(255));

    // F1 button
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, MCU_NOTE_F1, 0x7F, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_FUNCTION_OFFSET);
    QCOMPARE(value, uchar(255));

    // Unknown note 0xFF should fail
    QVERIFY(!MackieControlProtocol::mackieToInput(
        MIDI_NOTE_ON, 0xFF, 0x7F, &channel, &value));
}

void Midi_Test::mackieToInput_vpots()
{
    quint32 channel = 0;
    uchar value = 0;

    // VPot 1 clockwise 1 click: CC 16, data2=0x01
    // value = 127 + 1 = 128
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x01, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VPOT_OFFSET + 0));
    QCOMPARE(value, uchar(128));

    // VPot 1 clockwise max speed: CC 16, data2=0x0F
    // value = 127 + 15 = 142
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x0F, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VPOT_OFFSET + 0));
    QCOMPARE(value, uchar(142));

    // VPot 1 counter-clockwise 1 click: CC 16, data2=0x41
    // value = 127 - (0x41 - 0x40) = 127 - 1 = 126
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x41, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VPOT_OFFSET + 0));
    QCOMPARE(value, uchar(126));

    // VPot 1 counter-clockwise max speed: CC 16, data2=0x4F
    // value = 127 - (0x4F - 0x40) = 127 - 15 = 112
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x4F, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VPOT_OFFSET + 0));
    QCOMPARE(value, uchar(112));

    // VPot 8: CC 23, data2=0x01
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE + 7, 0x01, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VPOT_OFFSET + 7));
    QCOMPARE(value, uchar(128));

    // Unrelated CC (CC 10) should fail
    QVERIFY(!MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, 10, 0x01, &channel, &value));
}

void Midi_Test::mackieToInput_jogWheel()
{
    quint32 channel = 0;
    uchar value = 0;

    // Jog clockwise: CC 60, data2=0x01
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_JOG_WHEEL, 0x01, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_JOG_OFFSET);
    QCOMPARE(value, uchar(128));

    // Jog counter-clockwise: CC 60, data2=0x41
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CONTROL_CHANGE, MCU_CC_JOG_WHEEL, 0x41, &channel, &value));
    QCOMPARE(channel, (quint32)MACKIE_JOG_OFFSET);
    QCOMPARE(value, uchar(126));
}

void Midi_Test::mackieToInput_vuMeters()
{
    quint32 channel = 0;
    uchar value = 0;

    // VU channel 0, level 14 (max/overload): data1 = (0 << 4) | 14 = 0x0E
    // value = (14 * 255) / 14 = 255
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x0E, 0, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(value, uchar(255));

    // VU channel 0, level 12: data1 = 0x0C
    // value = (12 * 255) / 14 = 218
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x0C, 0, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(value, uchar(218));

    // VU channel 0, level 0: data1 = 0x00
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x00, 0, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(value, uchar(0));

    // VU channel 1, level 6: data1 = (1 << 4) | 6 = 0x16
    // value = (6 * 255) / 14 = 109
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x16, 0, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VU_OFFSET + 1));
    QCOMPARE(value, uchar(109));

    // VU channel 7, level 14 (overload): data1 = (7 << 4) | 14 = 0x7E
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x7E, 0, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VU_OFFSET + 7));
    QCOMPARE(value, uchar(255));

    // VU channel 0, level 0xF (clear overload): data1 = 0x0F → value=0
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x0F, 0, &channel, &value));
    QCOMPARE(channel, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(value, uchar(0));
}

/****************************************************************************
 * Mackie Control Protocol: Feedback conversion tests
 ****************************************************************************/

void Midi_Test::feedbackToMackie_faders()
{
    uchar cmd = 0, data1 = 0, data2 = 0;

    // Fader 1, value=0 → Pitch Bend ch 0, zero
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_FADER_OFFSET + 0, 0, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL | 0));
    QCOMPARE(data1, uchar(0));
    QCOMPARE(data2, uchar(0));

    // Fader 1, value=255 → maximum 14-bit pitch bend
    // 255 * 16383 / 255 = 16383 = 0x3FFF, LSB = 0x7F, MSB = 0x7F
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_FADER_OFFSET + 0, 255, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL | 0));
    QCOMPARE(data1, uchar(0x7F));
    QCOMPARE(data2, uchar(0x7F));

    // Master fader (ch 8), value=128
    // 128 * 16383 / 255 = 8223 = 0x201F, LSB = 0x1F, MSB = 0x40
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_FADER_OFFSET + 8, 128, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL | 8));
    QCOMPARE(data1, uchar(0x1F));
    QCOMPARE(data2, uchar(0x40));
}

void Midi_Test::feedbackToMackie_buttons()
{
    uchar cmd = 0, data1 = 0, data2 = 0;

    // REC 1 LED on: channel=25, value=255
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_REC_OFFSET, 255, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_NOTE_ON));
    QCOMPARE(data1, uchar(MCU_NOTE_REC_BASE));
    QCOMPARE(data2, uchar(0x7F));

    // REC 1 LED off: value=0
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_REC_OFFSET, 0, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_NOTE_ON));
    QCOMPARE(data1, uchar(MCU_NOTE_REC_BASE));
    QCOMPARE(data2, uchar(0x00));

    // PLAY LED on: channel=69 (MACKIE_TRANSPORT_OFFSET + 3)
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_TRANSPORT_OFFSET + 3, 255, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_NOTE_ON));
    QCOMPARE(data1, uchar(MCU_NOTE_PLAY));
    QCOMPARE(data2, uchar(0x7F));

    // SOLO 4 LED on
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_SOLO_OFFSET + 3, 255, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_NOTE_ON));
    QCOMPARE(data1, uchar(MCU_NOTE_SOLO_BASE + 3));
    QCOMPARE(data2, uchar(0x7F));

    // Intermediate value (128) still maps to LED on
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_REC_OFFSET, 128, &cmd, &data1, &data2));
    QCOMPARE(data2, uchar(0x7F));
}

void Midi_Test::feedbackToMackie_vpotLeds()
{
    uchar cmd = 0, data1 = 0, data2 = 0;

    // VPot LED 1 at zero: channel=200, value=0
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VPOT_LED_OFFSET + 0, 0, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE));
    QCOMPARE(data1, uchar(MCU_CC_VPOT_LED_BASE));
    // position = (0 * 11) / 255 = 0, mode=SINGLE → encodeVPotLed(0, 0) = 0x00
    QCOMPARE(data2, uchar(0x00));

    // VPot LED 1 at max: channel=200, value=255
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VPOT_LED_OFFSET + 0, 255, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE));
    QCOMPARE(data1, uchar(MCU_CC_VPOT_LED_BASE));
    // position = (255 * 11) / 255 = 11, mode=SINGLE → encodeVPotLed(11, 0) = 0x0B
    QCOMPARE(data2, uchar(0x0B));

    // VPot LED 8: channel=207, value=128
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VPOT_LED_OFFSET + 7, 128, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE));
    QCOMPARE(data1, uchar(MCU_CC_VPOT_LED_BASE + 7));
    // position = (128 * 11) / 255 = 5
    uchar expectedPos = (128 * 11) / 255;
    QCOMPARE(data2, MackieControlProtocol::encodeVPotLed(expectedPos, MCU_VPOT_MODE_SINGLE));
}

void Midi_Test::feedbackToMackie_vuMeters()
{
    uchar cmd = 0, data1 = 0, data2 = 0;

    // VU ch 0, value=255 (max) → level = (255 * 14) / 255 = 14
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VU_OFFSET + 0, 255, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(data1, uchar((0 << 4) | 14));
    QCOMPARE(data2, uchar(0));

    // VU ch 1, value=0 → level=0
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VU_OFFSET + 1, 0, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(data1, uchar((1 << 4) | 0));
    QCOMPARE(data2, uchar(0));

    // VU ch 3, value=128 → level = (128 * 14) / 255 = 7
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VU_OFFSET + 3, 128, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(data1, uchar((3 << 4) | 7));
}

void Midi_Test::feedbackToMackie_7segment()
{
    uchar cmd = 0, data1 = 0, data2 = 0;

    // 7-segment digit 0: channel=208, value=0x35 (digit '5')
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_7SEG_OFFSET + 0, 0x35, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE));
    QCOMPARE(data1, uchar(MCU_CC_7SEG_BASE + 0));
    QCOMPARE(data2, uchar(0x35));

    // 7-segment digit 11: channel=219, value=0x39 (digit '9')
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_7SEG_OFFSET + 11, 0x39, &cmd, &data1, &data2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE));
    QCOMPARE(data1, uchar(MCU_CC_7SEG_BASE + 11));
    QCOMPARE(data2, uchar(0x39));

    // High bit masked: value=0xFF → data2 = 0xFF & 0x7F = 0x7F
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_7SEG_OFFSET + 0, 0xFF, &cmd, &data1, &data2));
    QCOMPARE(data2, uchar(0x7F));
}

/****************************************************************************
 * Mackie Control Protocol: Note ↔ Channel round-trip
 ****************************************************************************/

void Midi_Test::noteToChannel_roundTrip()
{
    // Test all per-channel button groups (8 buttons each)
    struct { uchar baseNote; quint32 baseChannel; int count; } groups[] = {
        { MCU_NOTE_REC_BASE,          MACKIE_REC_OFFSET,          8 },
        { MCU_NOTE_SOLO_BASE,         MACKIE_SOLO_OFFSET,         8 },
        { MCU_NOTE_MUTE_BASE,         MACKIE_MUTE_OFFSET,         8 },
        { MCU_NOTE_SELECT_BASE,       MACKIE_SELECT_OFFSET,       8 },
        { MCU_NOTE_VPOT_SW_BASE,      MACKIE_VPOT_PUSH_OFFSET,    8 },
        { MCU_NOTE_FADER_TOUCH_BASE,  MACKIE_FADER_TOUCH_OFFSET,  9 },  // 8 + master
        { MCU_NOTE_F1,                MACKIE_FUNCTION_OFFSET,      8 },
    };

    for (size_t g = 0; g < sizeof(groups) / sizeof(groups[0]); g++)
    {
        for (int i = 0; i < groups[g].count; i++)
        {
            uchar note = groups[g].baseNote + i;
            quint32 ch = MackieControlProtocol::noteToChannel(note);
            QCOMPARE(ch, groups[g].baseChannel + (quint32)i);

            // Round-trip: channelToNote should give back the original note
            uchar backNote = MackieControlProtocol::channelToNote(ch);
            QCOMPARE(backNote, note);
        }
    }

    // Test individual buttons (transport, assignment, etc.)
    struct { uchar note; quint32 expectedChannel; } singles[] = {
        { MCU_NOTE_REWIND,           MACKIE_TRANSPORT_OFFSET + 0 },
        { MCU_NOTE_FORWARD,          MACKIE_TRANSPORT_OFFSET + 1 },
        { MCU_NOTE_STOP,             MACKIE_TRANSPORT_OFFSET + 2 },
        { MCU_NOTE_PLAY,             MACKIE_TRANSPORT_OFFSET + 3 },
        { MCU_NOTE_RECORD,           MACKIE_TRANSPORT_OFFSET + 4 },
        { MCU_NOTE_CYCLE,            MACKIE_TRANSPORT_OFFSET + 5 },
        { MCU_NOTE_ASSIGN_TRACK,     MACKIE_ASSIGNMENT_OFFSET + 0 },
        { MCU_NOTE_ASSIGN_INSTRUMENT,MACKIE_ASSIGNMENT_OFFSET + 5 },
        { MCU_NOTE_BANK_LEFT,        MACKIE_BANK_OFFSET + 0 },
        { MCU_NOTE_CH_RIGHT,         MACKIE_BANK_OFFSET + 3 },
        { MCU_NOTE_SHIFT,            MACKIE_MODIFIER_OFFSET + 0 },
        { MCU_NOTE_ALT,              MACKIE_MODIFIER_OFFSET + 3 },
        { MCU_NOTE_READ,             MACKIE_AUTOMATION_OFFSET + 0 },
        { MCU_NOTE_GROUP,            MACKIE_AUTOMATION_OFFSET + 5 },
        { MCU_NOTE_SAVE,             MACKIE_UTILITY_OFFSET + 0 },
        { MCU_NOTE_ENTER,            MACKIE_UTILITY_OFFSET + 3 },
        { MCU_NOTE_CURSOR_UP,        MACKIE_CURSOR_OFFSET + 0 },
        { MCU_NOTE_SCRUB,            MACKIE_CURSOR_OFFSET + 5 },
        { MCU_NOTE_FLIP,             MACKIE_MISC_OFFSET + 0 },
        { MCU_NOTE_USER_B,           MACKIE_MISC_OFFSET + 11 },
    };

    for (size_t i = 0; i < sizeof(singles) / sizeof(singles[0]); i++)
    {
        quint32 ch = MackieControlProtocol::noteToChannel(singles[i].note);
        QCOMPARE(ch, singles[i].expectedChannel);

        // Round-trip
        uchar backNote = MackieControlProtocol::channelToNote(ch);
        QCOMPARE(backNote, singles[i].note);
    }

    // Unknown note should return UINT_MAX
    QCOMPARE(MackieControlProtocol::noteToChannel(0xFF), (quint32)UINT_MAX);
    QCOMPARE(MackieControlProtocol::noteToChannel(0x71), (quint32)UINT_MAX);

    // Unknown channel should return 0xFF
    QCOMPARE(MackieControlProtocol::channelToNote(999), uchar(0xFF));
    QCOMPARE(MackieControlProtocol::channelToNote(MACKIE_FADER_OFFSET), uchar(0xFF)); // faders aren't buttons
}

/****************************************************************************
 * Mackie Control Protocol: Round-trip tests
 ****************************************************************************/

void Midi_Test::faderRoundTrip()
{
    // For every 8-bit value, feedbackToMackie → mackieToInput should round-trip
    // with at most 1 quantization error (14-bit ↔ 8-bit)
    for (int val = 0; val <= 255; val++)
    {
        uchar cmd, d1, d2;
        QVERIFY(MackieControlProtocol::feedbackToMackie(
            MACKIE_FADER_OFFSET + 0, (uchar)val, &cmd, &d1, &d2));

        quint32 ch;
        uchar inVal;
        QVERIFY(MackieControlProtocol::mackieToInput(cmd, d1, d2, &ch, &inVal));
        QCOMPARE(ch, (quint32)(MACKIE_FADER_OFFSET + 0));
        QVERIFY2(qAbs((int)inVal - val) <= 1,
                 qPrintable(QString("val=%1 inVal=%2").arg(val).arg(inVal)));
    }
}

void Midi_Test::vuMeterRoundTrip()
{
    // For every 8-bit value, feedbackToMackie → mackieToInput should round-trip
    // with bounded quantization error (0-14 levels ↔ 0-255 range)
    for (int val = 0; val <= 255; val++)
    {
        uchar cmd, d1, d2;
        QVERIFY(MackieControlProtocol::feedbackToMackie(
            MACKIE_VU_OFFSET + 0, (uchar)val, &cmd, &d1, &d2));

        quint32 ch;
        uchar inVal;
        QVERIFY(MackieControlProtocol::mackieToInput(cmd, d1, d2, &ch, &inVal));
        QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 0));
        // VU has only 15 levels (0-14) so quantization error can be up to 255/14 ≈ 18
        QVERIFY2(qAbs((int)inVal - val) <= 19,
                 qPrintable(QString("val=%1 inVal=%2").arg(val).arg(inVal)));
    }
}

/****************************************************************************
 * Mackie Control Protocol: VPot LED encoding
 ****************************************************************************/

void Midi_Test::encodeVPotLed()
{
    // position=5, mode=SINGLE, centerLed=false → 0x05
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_SINGLE, false),
             uchar(0x05));

    // position=5, mode=BOOST_CUT → 0x15 (mode=1 << 4 = 0x10)
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_BOOST_CUT, false),
             uchar(0x15));

    // position=5, mode=WRAP → 0x25 (mode=2 << 4 = 0x20)
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_WRAP, false),
             uchar(0x25));

    // position=5, mode=SPREAD → 0x35 (mode=3 << 4 = 0x30)
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_SPREAD, false),
             uchar(0x35));

    // position=5, mode=SINGLE, centerLed=true → 0x45 (center=0x40)
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_SINGLE, true),
             uchar(0x45));

    // position=0 (off), mode=SINGLE → 0x00
    QCOMPARE(MackieControlProtocol::encodeVPotLed(0, MCU_VPOT_MODE_SINGLE, false),
             uchar(0x00));

    // position=11 (max), mode=SINGLE → 0x0B
    QCOMPARE(MackieControlProtocol::encodeVPotLed(11, MCU_VPOT_MODE_SINGLE, false),
             uchar(0x0B));

    // All options: position=11, mode=SPREAD, centerLed=true → 0x3B | 0x40 = 0x7B
    QCOMPARE(MackieControlProtocol::encodeVPotLed(11, MCU_VPOT_MODE_SPREAD, true),
             uchar(0x7B));
}

/****************************************************************************
 * Mackie Control Protocol: Handshake challenge/response algorithm
 * (Inline test to avoid pulling in MackieControlHandler dependencies)
 ****************************************************************************/

// Duplicate of the algorithm from mackiecontrolhandler.cpp for standalone testing
static void testChallengeResponse(const uchar c[4], uchar r[4])
{
    r[0] = 0x7F & (c[0] + (c[1] ^ 0x0A) - c[3]);
    r[1] = 0x7F & ((c[2] >> 4) ^ (c[0] + c[3]));
    r[2] = 0x7F & ((c[3] - (c[2] << 2)) ^ (c[0] | c[1]));
    r[3] = 0x7F & (c[1] - c[2] + (0xF0 ^ (c[3] << 4)));
}

void Midi_Test::challengeResponseAlgorithm()
{
    uchar challenge[4];
    uchar response[4];

    // Test with all zeros
    challenge[0] = 0; challenge[1] = 0; challenge[2] = 0; challenge[3] = 0;
    testChallengeResponse(challenge, response);
    // r[0] = 0x7F & (0 + (0 ^ 0x0A) - 0) = 0x7F & 0x0A = 0x0A
    QCOMPARE(response[0], uchar(0x0A));
    // r[1] = 0x7F & ((0 >> 4) ^ (0 + 0)) = 0x7F & 0 = 0x00
    QCOMPARE(response[1], uchar(0x00));
    // r[2] = 0x7F & (0 - (0 << 2) ^ (0 | 0)) = 0x7F & 0 = 0x00
    QCOMPARE(response[2], uchar(0x00));
    // r[3] = 0x7F & (0 - 0 + (0xF0 ^ (0 << 4))) = 0x7F & (0xF0) = 0x70
    QCOMPARE(response[3], uchar(0x70));

    // All response bytes must be valid MIDI data bytes (0x00-0x7F)
    for (int i = 0; i < 4; i++)
        QVERIFY(response[i] <= 0x7F);

    // Test with a known non-trivial input
    challenge[0] = 0x10; challenge[1] = 0x20; challenge[2] = 0x30; challenge[3] = 0x40;
    testChallengeResponse(challenge, response);
    // Verify all response bytes are valid MIDI data bytes
    for (int i = 0; i < 4; i++)
        QVERIFY(response[i] <= 0x7F);

    // Test with max values (0x7F)
    challenge[0] = 0x7F; challenge[1] = 0x7F; challenge[2] = 0x7F; challenge[3] = 0x7F;
    testChallengeResponse(challenge, response);
    // All response bytes must be within MIDI range
    for (int i = 0; i < 4; i++)
        QVERIFY(response[i] <= 0x7F);
}

QTEST_MAIN(Midi_Test)
