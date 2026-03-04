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
#include <QSignalSpy>
#include <QSet>
#include <climits>

#define private public
#include "midi_test.h"
#include "midiprotocol.h"
#include "mackiecontrolprotocol.h"
#include "mackiecontrolhandler.h"
#include "midioutputdevice.h"
#undef private

/****************************************************************************
 * Mock MidiOutputDevice for handler testing
 ****************************************************************************/

class MockMidiOutputDevice : public MidiOutputDevice
{
public:
    MockMidiOutputDevice()
        : MidiOutputDevice(QVariant("mock-uid"), "MockMCU")
        , m_isOpen(false)
    {
    }

    bool open() override { m_isOpen = true; return true; }
    void close() override { m_isOpen = false; }
    bool isOpen() const override { return m_isOpen; }

    void writeChannel(ushort channel, uchar value) override
    { Q_UNUSED(channel) Q_UNUSED(value) }

    void writeUniverse(const QByteArray& universe) override
    { Q_UNUSED(universe) }

    void writeFeedback(uchar cmd, uchar data1, uchar data2) override
    {
        FeedbackMsg msg;
        msg.cmd = cmd; msg.data1 = data1; msg.data2 = data2;
        feedbackMsgs.append(msg);
    }

    void writeSysEx(QByteArray message) override
    { sysExMsgs.append(message); }

    struct FeedbackMsg { uchar cmd, data1, data2; };
    QList<FeedbackMsg> feedbackMsgs;
    QList<QByteArray> sysExMsgs;
    bool m_isOpen;

    void clear() { feedbackMsgs.clear(); sysExMsgs.clear(); }
};

/****************************************************************************
 * Original MIDI protocol tests
 ****************************************************************************/

void Midi_Test::midiToInput()
{
    quint32 channel = 0;
    uchar value = 0;
    uchar midiChannel = 7;
    uchar cmd = MIDI_NOTE_ON | midiChannel;
    QLCMIDIProtocol::midiToInput(cmd, 10, 127, midiChannel, &channel, &value);
    QCOMPARE(channel, 138U);
    QCOMPARE(value, uchar(255U));
}

/****************************************************************************
 * Mackie Control Protocol: Input conversion tests
 ****************************************************************************/

void Midi_Test::mackieToInput_faders()
{
    quint32 ch = 0; uchar val = 0;

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|0, 0x00, 0x40, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_FADER_OFFSET+0)); QCOMPARE(val, uchar(128));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|0, 0x00, 0x00, &ch, &val));
    QCOMPARE(val, uchar(0));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|0, 0x7F, 0x7F, &ch, &val));
    QCOMPARE(val, uchar(255));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|8, 0x00, 0x40, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_FADER_OFFSET+8)); QCOMPARE(val, uchar(128));

    QVERIFY(!MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|9, 0x00, 0x40, &ch, &val));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|4, 0x40, 0x40, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_FADER_OFFSET+4)); QCOMPARE(val, uchar(129));
}

void Midi_Test::mackieToInput_buttons()
{
    quint32 ch = 0; uchar val = 0;

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_REC_BASE, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_REC_OFFSET); QCOMPARE(val, uchar(255));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_REC_BASE, 0x00, &ch, &val));
    QCOMPARE(val, uchar(0));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_OFF, MCU_NOTE_SOLO_BASE, 0x00, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_SOLO_OFFSET); QCOMPARE(val, uchar(0));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_PLAY, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_TRANSPORT_OFFSET+3)); QCOMPARE(val, uchar(255));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_MUTE_BASE+7, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_MUTE_OFFSET+7));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_SELECT_BASE, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_SELECT_OFFSET);

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_VPOT_SW_BASE, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_VPOT_PUSH_OFFSET);

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_FADER_TOUCH_BASE, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_FADER_TOUCH_OFFSET);

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_FADER_TOUCH_MASTER, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_FADER_TOUCH_OFFSET+8));

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_F1, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_FUNCTION_OFFSET);

    QVERIFY(!MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, 0xFF, 0x7F, &ch, &val));
}

void Midi_Test::mackieToInput_vpots()
{
    quint32 ch = 0; uchar val = 0;
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x01, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VPOT_OFFSET+0)); QCOMPARE(val, uchar(128));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x0F, &ch, &val));
    QCOMPARE(val, uchar(142));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x41, &ch, &val));
    QCOMPARE(val, uchar(126));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x4F, &ch, &val));
    QCOMPARE(val, uchar(112));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE+7, 0x01, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VPOT_OFFSET+7)); QCOMPARE(val, uchar(128));
    QVERIFY(!MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, 10, 0x01, &ch, &val));
}

void Midi_Test::mackieToInput_jogWheel()
{
    quint32 ch = 0; uchar val = 0;
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_JOG_WHEEL, 0x01, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_JOG_OFFSET); QCOMPARE(val, uchar(128));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_JOG_WHEEL, 0x41, &ch, &val));
    QCOMPARE(val, uchar(126));
}

void Midi_Test::mackieToInput_vuMeters()
{
    quint32 ch = 0;
    uchar val = 0;

    // VU channel 0, level 14 (max): data1 = (0 << 4) | 14 = 0x0E
    // value = (14 * 255) / 14 = 255
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x0E, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(val, uchar(255));

    // VU channel 0, level 12: data1 = 0x0C
    // value = (12 * 255) / 14 = 218
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x0C, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(val, uchar(218));

    // VU channel 0, level 0: data1 = 0x00
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x00, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(val, uchar(0));

    // VU channel 1, level 6: data1 = (1 << 4) | 6 = 0x16
    // value = (6 * 255) / 14 = 109
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x16, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 1));
    QCOMPARE(val, uchar(109));

    // VU channel 7, level 14 (overload): data1 = (7 << 4) | 14 = 0x7E
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x7E, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 7));
    QCOMPARE(val, uchar(255));

    // VU channel 0, level 0xF (clear overload): data1 = 0x0F → value=0
    QVERIFY(MackieControlProtocol::mackieToInput(
        MIDI_CHANNEL_AFTERTOUCH, 0x0F, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET + 0));
    QCOMPARE(val, uchar(0));
}

/****************************************************************************
 * Feedback conversion tests
 ****************************************************************************/

void Midi_Test::feedbackToMackie_faders()
{
    uchar cmd=0, d1=0, d2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_FADER_OFFSET+0, 0, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL|0)); QCOMPARE(d1, uchar(0)); QCOMPARE(d2, uchar(0));

    // Fader 1, value=255 → maximum 14-bit pitch bend
    // 255 * 16383 / 255 = 16383 = 0x3FFF, LSB = 0x7F, MSB = 0x7F
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_FADER_OFFSET + 0, 255, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL | 0));
    QCOMPARE(d1, uchar(0x7F));
    QCOMPARE(d2, uchar(0x7F));

    // Master fader (ch 8), value=128
    // 128 * 16383 / 255 = 8223 = 0x201F, LSB = 0x1F, MSB = 0x40
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_FADER_OFFSET + 8, 128, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL | 8));
    QCOMPARE(d1, uchar(0x1F));
    QCOMPARE(d2, uchar(0x40));
}

void Midi_Test::feedbackToMackie_buttons()
{
    uchar cmd=0, d1=0, d2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_REC_OFFSET, 255, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_NOTE_ON)); QCOMPARE(d1, uchar(MCU_NOTE_REC_BASE)); QCOMPARE(d2, uchar(0x7F));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_REC_OFFSET, 0, &cmd, &d1, &d2));
    QCOMPARE(d2, uchar(0x00));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_TRANSPORT_OFFSET+3, 255, &cmd, &d1, &d2));
    QCOMPARE(d1, uchar(MCU_NOTE_PLAY)); QCOMPARE(d2, uchar(0x7F));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_SOLO_OFFSET+3, 255, &cmd, &d1, &d2));
    QCOMPARE(d1, uchar(MCU_NOTE_SOLO_BASE+3));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_REC_OFFSET, 128, &cmd, &d1, &d2));
    QCOMPARE(d2, uchar(0x7F));
}

void Midi_Test::feedbackToMackie_vpotLeds()
{
    uchar cmd=0, d1=0, d2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_VPOT_LED_OFFSET+0, 0, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE)); QCOMPARE(d1, uchar(MCU_CC_VPOT_LED_BASE));
    QCOMPARE(d2, uchar(0x00));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_VPOT_LED_OFFSET+0, 255, &cmd, &d1, &d2));
    QCOMPARE(d2, uchar(0x0B));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_VPOT_LED_OFFSET+7, 128, &cmd, &d1, &d2));
    QCOMPARE(d1, uchar(MCU_CC_VPOT_LED_BASE+7));
    uchar ep = (128 * 11) / 255;
    QCOMPARE(d2, MackieControlProtocol::encodeVPotLed(ep, MCU_VPOT_MODE_SINGLE));
}

void Midi_Test::feedbackToMackie_vuMeters()
{
    uchar cmd=0, d1=0, d2=0;
    // VU ch 0, value=255 (max) → level = (255 * 14) / 255 = 14
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VU_OFFSET + 0, 255, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(d1, uchar((0 << 4) | 14));
    QCOMPARE(d2, uchar(0));

    // VU ch 1, value=0 → level=0
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VU_OFFSET + 1, 0, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(d1, uchar((1 << 4) | 0));
    QCOMPARE(d2, uchar(0));

    // VU ch 3, value=128 → level = (128 * 14) / 255 = 7
    QVERIFY(MackieControlProtocol::feedbackToMackie(
        MACKIE_VU_OFFSET + 3, 128, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(d1, uchar((3 << 4) | 7));
}

void Midi_Test::feedbackToMackie_7segment()
{
    uchar cmd=0, d1=0, d2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_7SEG_OFFSET+0, 0x35, &cmd, &d1, &d2));
    QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE)); QCOMPARE(d1, uchar(MCU_CC_7SEG_BASE)); QCOMPARE(d2, uchar(0x35));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_7SEG_OFFSET+11, 0x39, &cmd, &d1, &d2));
    QCOMPARE(d1, uchar(MCU_CC_7SEG_BASE+11)); QCOMPARE(d2, uchar(0x39));

    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_7SEG_OFFSET+0, 0xFF, &cmd, &d1, &d2));
    QCOMPARE(d2, uchar(0x7F));
}

/****************************************************************************
 * Note <-> Channel round-trip
 ****************************************************************************/

void Midi_Test::noteToChannel_roundTrip()
{
    struct { uchar baseNote; quint32 baseCh; int cnt; } groups[] = {
        {MCU_NOTE_REC_BASE, MACKIE_REC_OFFSET, 8},
        {MCU_NOTE_SOLO_BASE, MACKIE_SOLO_OFFSET, 8},
        {MCU_NOTE_MUTE_BASE, MACKIE_MUTE_OFFSET, 8},
        {MCU_NOTE_SELECT_BASE, MACKIE_SELECT_OFFSET, 8},
        {MCU_NOTE_VPOT_SW_BASE, MACKIE_VPOT_PUSH_OFFSET, 8},
        {MCU_NOTE_FADER_TOUCH_BASE, MACKIE_FADER_TOUCH_OFFSET, 9},
        {MCU_NOTE_F1, MACKIE_FUNCTION_OFFSET, 8},
    };
    for (size_t g = 0; g < sizeof(groups)/sizeof(groups[0]); g++)
        for (int i = 0; i < groups[g].cnt; i++)
        {
            uchar n = groups[g].baseNote + i;
            quint32 c = MackieControlProtocol::noteToChannel(n);
            QCOMPARE(c, groups[g].baseCh + (quint32)i);
            QCOMPARE(MackieControlProtocol::channelToNote(c), n);
        }

    struct { uchar n; quint32 c; } singles[] = {
        {MCU_NOTE_REWIND, MACKIE_TRANSPORT_OFFSET+0}, {MCU_NOTE_FORWARD, MACKIE_TRANSPORT_OFFSET+1},
        {MCU_NOTE_STOP, MACKIE_TRANSPORT_OFFSET+2}, {MCU_NOTE_PLAY, MACKIE_TRANSPORT_OFFSET+3},
        {MCU_NOTE_RECORD, MACKIE_TRANSPORT_OFFSET+4}, {MCU_NOTE_CYCLE, MACKIE_TRANSPORT_OFFSET+5},
        {MCU_NOTE_ASSIGN_TRACK, MACKIE_ASSIGNMENT_OFFSET+0}, {MCU_NOTE_ASSIGN_INSTRUMENT, MACKIE_ASSIGNMENT_OFFSET+5},
        {MCU_NOTE_BANK_LEFT, MACKIE_BANK_OFFSET+0}, {MCU_NOTE_CH_RIGHT, MACKIE_BANK_OFFSET+3},
        {MCU_NOTE_SHIFT, MACKIE_MODIFIER_OFFSET+0}, {MCU_NOTE_ALT, MACKIE_MODIFIER_OFFSET+3},
        {MCU_NOTE_READ, MACKIE_AUTOMATION_OFFSET+0}, {MCU_NOTE_GROUP, MACKIE_AUTOMATION_OFFSET+5},
        {MCU_NOTE_SAVE, MACKIE_UTILITY_OFFSET+0}, {MCU_NOTE_ENTER, MACKIE_UTILITY_OFFSET+3},
        {MCU_NOTE_CURSOR_UP, MACKIE_CURSOR_OFFSET+0}, {MCU_NOTE_SCRUB, MACKIE_CURSOR_OFFSET+5},
        {MCU_NOTE_FLIP, MACKIE_MISC_OFFSET+0}, {MCU_NOTE_USER_B, MACKIE_MISC_OFFSET+11},
    };
    for (size_t i = 0; i < sizeof(singles)/sizeof(singles[0]); i++)
    {
        QCOMPARE(MackieControlProtocol::noteToChannel(singles[i].n), singles[i].c);
        QCOMPARE(MackieControlProtocol::channelToNote(singles[i].c), singles[i].n);
    }

    QCOMPARE(MackieControlProtocol::noteToChannel(0xFF), (quint32)UINT_MAX);
    QCOMPARE(MackieControlProtocol::noteToChannel(0x71), (quint32)UINT_MAX);
    QCOMPARE(MackieControlProtocol::channelToNote(999), uchar(0xFF));
    QCOMPARE(MackieControlProtocol::channelToNote(MACKIE_FADER_OFFSET), uchar(0xFF));
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
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_SINGLE, false), uchar(0x05));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_BOOST_CUT, false), uchar(0x15));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_WRAP, false), uchar(0x25));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_SPREAD, false), uchar(0x35));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(5, MCU_VPOT_MODE_SINGLE, true), uchar(0x45));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(0, MCU_VPOT_MODE_SINGLE, false), uchar(0x00));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(11, MCU_VPOT_MODE_SINGLE, false), uchar(0x0B));
    QCOMPARE(MackieControlProtocol::encodeVPotLed(11, MCU_VPOT_MODE_SPREAD, true), uchar(0x7B));
}

/****************************************************************************
 * Challenge/response algorithm
 ****************************************************************************/

void Midi_Test::challengeResponseAlgorithm()
{
    uchar c[4], r[4];

    c[0]=0; c[1]=0; c[2]=0; c[3]=0;
    MackieControlHandler::computeChallengeResponse(c, r);
    QCOMPARE(r[0], uchar(0x0A)); QCOMPARE(r[1], uchar(0x00));
    QCOMPARE(r[2], uchar(0x00)); QCOMPARE(r[3], uchar(0x70));
    for (int i=0; i<4; i++) QVERIFY(r[i] <= 0x7F);

    c[0]=0x10; c[1]=0x20; c[2]=0x30; c[3]=0x40;
    MackieControlHandler::computeChallengeResponse(c, r);
    for (int i=0; i<4; i++) QVERIFY(r[i] <= 0x7F);

    c[0]=0x7F; c[1]=0x7F; c[2]=0x7F; c[3]=0x7F;
    MackieControlHandler::computeChallengeResponse(c, r);
    for (int i=0; i<4; i++) QVERIFY(r[i] <= 0x7F);
}

/****************************************************************************
 * Edge case tests
 ****************************************************************************/

void Midi_Test::mackieToInput_invalidMessages()
{
    quint32 ch=0; uchar val=0;
    QVERIFY(!MackieControlProtocol::mackieToInput(0x00, 0x00, 0x00, &ch, &val));
    QVERIFY(!MackieControlProtocol::mackieToInput(MIDI_PROGRAM_CHANGE, 0x00, 0x00, &ch, &val));
    QVERIFY(!MackieControlProtocol::mackieToInput(0xFF, 0x00, 0x00, &ch, &val));
}

void Midi_Test::feedbackToMackie_unmappedChannels()
{
    uchar cmd=0, d1=0, d2=0;
    QVERIFY(!MackieControlProtocol::feedbackToMackie(150, 128, &cmd, &d1, &d2));
    QVERIFY(!MackieControlProtocol::feedbackToMackie(250, 128, &cmd, &d1, &d2));
    QVERIFY(!MackieControlProtocol::feedbackToMackie(MACKIE_JOG_OFFSET, 128, &cmd, &d1, &d2));
}

void Midi_Test::mackieToInput_vpotBoundary()
{
    quint32 ch=0; uchar val=0;
    QVERIFY(!MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE+8, 0x01, &ch, &val));
    QVERIFY(!MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE-1, 0x01, &ch, &val));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x00, &ch, &val));
    QCOMPARE(val, uchar(127));
}

void Midi_Test::mackieToInput_vuOverRange()
{
    quint32 ch=0; uchar val=0;
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CHANNEL_AFTERTOUCH, 0x0E, 0, &ch, &val));
    QCOMPARE(ch, (quint32)(MACKIE_VU_OFFSET+0)); QCOMPARE(val, uchar(255));
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CHANNEL_AFTERTOUCH, 0x0F, 0, &ch, &val));
    QCOMPARE(val, uchar(0));  // 0x0F = clear overload → value 0
}

void Midi_Test::buttonRoundTrip()
{
    uchar bases[][2] = {{MCU_NOTE_REC_BASE,8},{MCU_NOTE_SOLO_BASE,8},{MCU_NOTE_MUTE_BASE,8},{MCU_NOTE_SELECT_BASE,8}};
    for (size_t g=0; g < sizeof(bases)/sizeof(bases[0]); g++)
        for (int i=0; i < bases[g][1]; i++)
        {
            uchar note = bases[g][0]+i;
            quint32 ch=0; uchar val=0;
            QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, note, 0x7F, &ch, &val));
            QCOMPARE(val, uchar(255));
            uchar fc=0,fd1=0,fd2=0;
            QVERIFY(MackieControlProtocol::feedbackToMackie(ch, 255, &fc, &fd1, &fd2));
            QCOMPARE(fc, uchar(MIDI_NOTE_ON)); QCOMPARE(fd1, note); QCOMPARE(fd2, uchar(0x7F));
            QVERIFY(MackieControlProtocol::feedbackToMackie(ch, 0, &fc, &fd1, &fd2));
            QCOMPARE(fd1, note); QCOMPARE(fd2, uchar(0x00));
        }
}

/****************************************************************************
 * Channel layout consistency
 ****************************************************************************/

void Midi_Test::channelLayoutNoOverlap()
{
    struct R { quint32 s, c; const char* n; };
    R ranges[] = {
        {MACKIE_FADER_OFFSET, MACKIE_FADER_COUNT, "Faders"},
        {MACKIE_VPOT_OFFSET, MACKIE_VPOT_COUNT, "VPots"},
        {MACKIE_VPOT_PUSH_OFFSET, MACKIE_VPOT_PUSH_COUNT, "VPotPush"},
        {MACKIE_REC_OFFSET, 8, "REC"}, {MACKIE_SOLO_OFFSET, 8, "SOLO"},
        {MACKIE_MUTE_OFFSET, 8, "MUTE"}, {MACKIE_SELECT_OFFSET, 8, "SELECT"},
        {MACKIE_FADER_TOUCH_OFFSET, 9, "Touch"},
        {MACKIE_TRANSPORT_OFFSET, 6, "Transport"},
        {MACKIE_FUNCTION_OFFSET, 8, "Function"},
        {MACKIE_ASSIGNMENT_OFFSET, 6, "Assign"},
        {MACKIE_BANK_OFFSET, 4, "Bank"},
        {MACKIE_MODIFIER_OFFSET, 4, "Mod"},
        {MACKIE_AUTOMATION_OFFSET, 6, "Auto"},
        {MACKIE_UTILITY_OFFSET, 4, "Util"},
        {MACKIE_CURSOR_OFFSET, 6, "Cursor"},
        {MACKIE_MISC_OFFSET, 20, "Misc"},
        {MACKIE_JOG_OFFSET, 1, "Jog"},
    };
    int n = sizeof(ranges)/sizeof(ranges[0]);
    for (int i=0; i<n; i++)
        for (int j=i+1; j<n; j++)
        {
            quint32 eA = ranges[i].s + ranges[i].c - 1;
            quint32 eB = ranges[j].s + ranges[j].c - 1;
            bool overlap = (ranges[i].s <= eB && ranges[j].s <= eA);
            if (overlap)
            {
                QString msg = QString("Overlap: %1 [%2-%3] and %4 [%5-%6]")
                    .arg(ranges[i].n).arg(ranges[i].s).arg(eA)
                    .arg(ranges[j].n).arg(ranges[j].s).arg(eB);
                QVERIFY2(!overlap, msg.toLatin1().constData());
            }
        }
}

void Midi_Test::allButtonNotesAreMapped()
{
    QSet<uchar> notes;
    for (int i=0;i<8;i++) {
        notes << (MCU_NOTE_REC_BASE+i) << (MCU_NOTE_SOLO_BASE+i) << (MCU_NOTE_MUTE_BASE+i)
              << (MCU_NOTE_SELECT_BASE+i) << (MCU_NOTE_VPOT_SW_BASE+i);
    }
    for (int i=0;i<=8;i++) notes << (MCU_NOTE_FADER_TOUCH_BASE+i);
    for (int i=0;i<8;i++) notes << (MCU_NOTE_F1+i);
    notes << MCU_NOTE_ASSIGN_TRACK << MCU_NOTE_ASSIGN_SEND << MCU_NOTE_ASSIGN_PAN
          << MCU_NOTE_ASSIGN_PLUGIN << MCU_NOTE_ASSIGN_EQ << MCU_NOTE_ASSIGN_INSTRUMENT;
    notes << MCU_NOTE_BANK_LEFT << MCU_NOTE_BANK_RIGHT << MCU_NOTE_CH_LEFT << MCU_NOTE_CH_RIGHT;
    notes << MCU_NOTE_FLIP << MCU_NOTE_GLOBAL_VIEW << MCU_NOTE_NAME_VALUE << MCU_NOTE_SMPTE_BEATS;
    notes << MCU_NOTE_SHIFT << MCU_NOTE_OPTION << MCU_NOTE_CONTROL << MCU_NOTE_ALT;
    notes << MCU_NOTE_READ << MCU_NOTE_WRITE << MCU_NOTE_TRIM << MCU_NOTE_TOUCH
          << MCU_NOTE_LATCH << MCU_NOTE_GROUP;
    notes << MCU_NOTE_SAVE << MCU_NOTE_UNDO << MCU_NOTE_CANCEL << MCU_NOTE_ENTER;
    notes << MCU_NOTE_MARKER << MCU_NOTE_NUDGE << MCU_NOTE_CYCLE << MCU_NOTE_DROP
          << MCU_NOTE_REPLACE << MCU_NOTE_CLICK << MCU_NOTE_SOLO_DEFEAT;
    notes << MCU_NOTE_REWIND << MCU_NOTE_FORWARD << MCU_NOTE_STOP << MCU_NOTE_PLAY << MCU_NOTE_RECORD;
    notes << MCU_NOTE_CURSOR_UP << MCU_NOTE_CURSOR_DOWN << MCU_NOTE_CURSOR_LEFT
          << MCU_NOTE_CURSOR_RIGHT << MCU_NOTE_ZOOM << MCU_NOTE_SCRUB;
    notes << MCU_NOTE_USER_A << MCU_NOTE_USER_B;
    notes << MCU_NOTE_MIDI_TRACKS << MCU_NOTE_INPUTS << MCU_NOTE_AUDIO_TRACKS
          << MCU_NOTE_AUDIO_INST << MCU_NOTE_AUX << MCU_NOTE_BUSSES
          << MCU_NOTE_OUTPUTS << MCU_NOTE_USER;

    for (uchar n : notes)
        QVERIFY2(MackieControlProtocol::noteToChannel(n) != UINT_MAX,
            QString("Note 0x%1 unmapped").arg(n,2,16,QChar('0')).toLatin1().constData());
}

void Midi_Test::globalViewSubButtonsMapping()
{
    struct { uchar note; quint32 off; } sb[] = {
        {MCU_NOTE_MIDI_TRACKS, MACKIE_MISC_OFFSET+12}, {MCU_NOTE_INPUTS, MACKIE_MISC_OFFSET+13},
        {MCU_NOTE_AUDIO_TRACKS, MACKIE_MISC_OFFSET+14}, {MCU_NOTE_AUDIO_INST, MACKIE_MISC_OFFSET+15},
        {MCU_NOTE_AUX, MACKIE_MISC_OFFSET+16}, {MCU_NOTE_BUSSES, MACKIE_MISC_OFFSET+17},
        {MCU_NOTE_OUTPUTS, MACKIE_MISC_OFFSET+18}, {MCU_NOTE_USER, MACKIE_MISC_OFFSET+19},
    };
    for (size_t i=0; i<sizeof(sb)/sizeof(sb[0]); i++)
    {
        quint32 ch = MackieControlProtocol::noteToChannel(sb[i].note);
        QCOMPARE(ch, sb[i].off);
        QCOMPARE(MackieControlProtocol::channelToNote(ch), sb[i].note);
        uchar cmd, d1, d2;
        QVERIFY(MackieControlProtocol::feedbackToMackie(ch, 255, &cmd, &d1, &d2));
        QCOMPARE(cmd, uchar(MIDI_NOTE_ON)); QCOMPARE(d1, sb[i].note); QCOMPARE(d2, uchar(0x7F));
    }
}

/****************************************************************************
 * MackieControlHandler tests
 ****************************************************************************/

void Midi_Test::handler_initialState()
{
    MackieControlHandler h;
    QCOMPARE(h.handshakeState(), MackieControlHandler::Idle);
    QCOMPARE(h.deviceId(), uchar(MCU_DEVICE_ID_MCU));
    QVERIFY(h.outputDevice() == nullptr);
}

void Midi_Test::handler_handshakeStateMachine()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.setDeviceId(MCU_DEVICE_ID_MCU);

    h.initiateHandshake();
    QCOMPARE(h.handshakeState(), MackieControlHandler::WaitingForChallenge);
    QCOMPARE(dev.sysExMsgs.size(), 1);
    QByteArray q = dev.sysExMsgs[0];
    QCOMPARE((uchar)q[0], uchar(0xF0));
    QCOMPARE((uchar)q[5], uchar(MCU_SYSEX_CMD_QUERY));

    // Simulate challenge
    QByteArray chal;
    chal.append((char)0xF0);
    chal.append((char)MCU_SYSEX_MANUFACTURER_1);
    chal.append((char)MCU_SYSEX_MANUFACTURER_2);
    chal.append((char)MCU_SYSEX_MANUFACTURER_3);
    chal.append((char)MCU_DEVICE_ID_MCU);
    chal.append((char)MCU_SYSEX_CMD_CHALLENGE);
    for (int i=0;i<7;i++) chal.append((char)(0x10+i));
    chal.append((char)0x01); chal.append((char)0x02);
    chal.append((char)0x03); chal.append((char)0x04);
    chal.append((char)0xF7);
    dev.clear();
    h.handleSysEx(chal);
    QCOMPARE(h.handshakeState(), MackieControlHandler::WaitingForConfirm);
    QCOMPARE(dev.sysExMsgs.size(), 1);
    QCOMPARE((uchar)dev.sysExMsgs[0][5], uchar(MCU_SYSEX_CMD_RESPONSE));

    // Simulate confirmation
    QSignalSpy spy(&h, SIGNAL(handshakeCompleted()));
    QByteArray conf;
    conf.append((char)0xF0);
    conf.append((char)MCU_SYSEX_MANUFACTURER_1);
    conf.append((char)MCU_SYSEX_MANUFACTURER_2);
    conf.append((char)MCU_SYSEX_MANUFACTURER_3);
    conf.append((char)MCU_DEVICE_ID_MCU);
    conf.append((char)MCU_SYSEX_CMD_CONFIRM);
    conf.append((char)0xF7);
    h.handleSysEx(conf);
    QCOMPARE(h.handshakeState(), MackieControlHandler::Connected);
    QCOMPARE(spy.count(), 1);
}

void Midi_Test::handler_challengeResponse()
{
    uchar c[4]={0x01,0x02,0x03,0x04}, r[4], r2[4];
    MackieControlHandler::computeChallengeResponse(c, r);
    for (int i=0;i<4;i++) QVERIFY(r[i] <= 0x7F);
    MackieControlHandler::computeChallengeResponse(c, r2);
    for (int i=0;i<4;i++) QCOMPARE(r[i], r2[i]);
}

void Midi_Test::handler_sysExValidation()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.initiateHandshake();
    dev.clear();

    QByteArray tooShort;
    tooShort.append((char)0xF0); tooShort.append((char)0x00); tooShort.append((char)0xF7);
    h.handleSysEx(tooShort);
    QCOMPARE(dev.sysExMsgs.size(), 0);
    QCOMPARE(h.handshakeState(), MackieControlHandler::WaitingForChallenge);

    QByteArray wrongMfr;
    wrongMfr.append((char)0xF0); wrongMfr.append((char)0x00);
    wrongMfr.append((char)0x01); wrongMfr.append((char)0x66);
    wrongMfr.append((char)MCU_DEVICE_ID_MCU); wrongMfr.append((char)MCU_SYSEX_CMD_CHALLENGE);
    wrongMfr.append((char)0xF7);
    h.handleSysEx(wrongMfr);
    QCOMPARE(h.handshakeState(), MackieControlHandler::WaitingForChallenge);

    QByteArray wrongDev;
    wrongDev.append((char)0xF0);
    wrongDev.append((char)MCU_SYSEX_MANUFACTURER_1); wrongDev.append((char)MCU_SYSEX_MANUFACTURER_2);
    wrongDev.append((char)MCU_SYSEX_MANUFACTURER_3);
    wrongDev.append((char)0x99); wrongDev.append((char)MCU_SYSEX_CMD_CHALLENGE);
    wrongDev.append((char)0xF7);
    h.handleSysEx(wrongDev);
    QCOMPARE(h.handshakeState(), MackieControlHandler::WaitingForChallenge);
}

void Midi_Test::handler_lcdOutput()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);

    h.updateLCD(0, 0, "Test");
    QCOMPARE(dev.sysExMsgs.size(), 1);
    QCOMPARE((uchar)dev.sysExMsgs[0][5], uchar(MCU_SYSEX_CMD_LCD));
    QCOMPARE((uchar)dev.sysExMsgs[0][6], uchar(0x00));
    QCOMPARE(dev.sysExMsgs[0].mid(7, 7), QByteArray("Test   "));

    dev.clear();
    h.updateLCD(1, 3, "Dimmer");
    QCOMPARE((uchar)dev.sysExMsgs[0][6], uchar(0x38 + 3*7));
}

void Midi_Test::handler_clearLCD()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.clearLCD();
    QCOMPARE(dev.sysExMsgs.size(), 2);
    QCOMPARE((uchar)dev.sysExMsgs[0][6], uchar(0x00));
    QCOMPARE((uchar)dev.sysExMsgs[1][6], uchar(0x38));
}

void Midi_Test::handler_7segOutput()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.update7Segment("123");
    QCOMPARE(dev.feedbackMsgs.size(), 12);
    for (int i=0;i<12;i++)
        QCOMPARE(dev.feedbackMsgs[i].cmd, uchar(MIDI_CONTROL_CHANGE));
}

void Midi_Test::handler_vuMeterOutput()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.updateVUMeter(0, 255);
    QCOMPARE(dev.feedbackMsgs.size(), 1);
    QCOMPARE(dev.feedbackMsgs[0].cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
    QCOMPARE(dev.feedbackMsgs[0].data1, uchar((0<<4)|14));
    dev.clear();
    h.updateVUMeter(8, 128);
    QCOMPARE(dev.feedbackMsgs.size(), 0);
    h.updateVUMeter(-1, 128);
    QCOMPARE(dev.feedbackMsgs.size(), 0);
}

void Midi_Test::handler_vuMeterClear()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.clearVUMeters();
    QCOMPARE(dev.feedbackMsgs.size(), 8);
    for (int i=0;i<8;i++) {
        QCOMPARE(dev.feedbackMsgs[i].cmd, uchar(MIDI_CHANNEL_AFTERTOUCH));
        QCOMPARE(dev.feedbackMsgs[i].data1, uchar(i<<4));
    }
}

void Midi_Test::handler_vpotLedOutput()
{
    MackieControlHandler h;
    MockMidiOutputDevice dev;
    h.setOutputDevice(&dev);
    h.updateVPotLed(0, 128, MCU_VPOT_MODE_SINGLE);
    QCOMPARE(dev.feedbackMsgs.size(), 1);
    QCOMPARE(dev.feedbackMsgs[0].cmd, uchar(MIDI_CONTROL_CHANGE));
    QCOMPARE(dev.feedbackMsgs[0].data1, uchar(MCU_CC_VPOT_LED_BASE));
    dev.clear();
    h.updateVPotLed(7, 255, MCU_VPOT_MODE_WRAP);
    QCOMPARE(dev.feedbackMsgs.size(), 1);
    QCOMPARE(dev.feedbackMsgs[0].data1, uchar(MCU_CC_VPOT_LED_BASE+7));
    dev.clear();
    h.updateVPotLed(8, 128);
    QCOMPARE(dev.feedbackMsgs.size(), 0);
}

void Midi_Test::handler_nullOutputDevice()
{
    MackieControlHandler h;
    h.updateLCD(0, 0, "Test");
    h.clearLCD();
    h.update7Segment("123");
    h.updateVUMeter(0, 128);
    h.clearVUMeters();
    h.updateVPotLed(0, 128);
    QVERIFY(true); // No crash = pass
}

/****************************************************************************
 * Input Profile verification
 ****************************************************************************/

void Midi_Test::inputProfile_loadAndVerifyChannels()
{
    // Verify protocol constant alignment with expected profile channel numbers
    QCOMPARE((quint32)MACKIE_FADER_OFFSET, (quint32)0);
    QCOMPARE((quint32)MACKIE_FADER_COUNT, (quint32)9);
    QCOMPARE((quint32)MACKIE_VPOT_OFFSET, (quint32)9);
    QCOMPARE((quint32)MACKIE_VPOT_COUNT, (quint32)8);
    QCOMPARE((quint32)MACKIE_REC_OFFSET, (quint32)25);
    QCOMPARE((quint32)MACKIE_SOLO_OFFSET, (quint32)33);
    QCOMPARE((quint32)MACKIE_MUTE_OFFSET, (quint32)41);
    QCOMPARE((quint32)MACKIE_SELECT_OFFSET, (quint32)49);
    QCOMPARE((quint32)MACKIE_FADER_TOUCH_OFFSET, (quint32)57);
    QCOMPARE((quint32)MACKIE_TRANSPORT_OFFSET, (quint32)66);
    QCOMPARE((quint32)MACKIE_FUNCTION_OFFSET, (quint32)72);
    QCOMPARE((quint32)MACKIE_MISC_OFFSET, (quint32)110);
    QCOMPARE((quint32)MACKIE_JOG_OFFSET, (quint32)130);
}

void Midi_Test::inputProfile_channelTypes()
{
    // Verify fader range is contiguous
    for (quint32 ch = MACKIE_FADER_OFFSET; ch < MACKIE_FADER_OFFSET+MACKIE_FADER_COUNT; ch++)
        QVERIFY(ch <= 8);

    // Verify VPot range
    for (quint32 ch = MACKIE_VPOT_OFFSET; ch < MACKIE_VPOT_OFFSET+MACKIE_VPOT_COUNT; ch++)
        QVERIFY(ch >= 9 && ch <= 16);

    // All button channel groups map to valid notes
    for (quint32 ch = MACKIE_REC_OFFSET; ch < MACKIE_REC_OFFSET+8; ch++)
        QVERIFY(MackieControlProtocol::channelToNote(ch) != 0xFF);
    for (quint32 ch = MACKIE_SOLO_OFFSET; ch < MACKIE_SOLO_OFFSET+8; ch++)
        QVERIFY(MackieControlProtocol::channelToNote(ch) != 0xFF);
    for (quint32 ch = MACKIE_MUTE_OFFSET; ch < MACKIE_MUTE_OFFSET+8; ch++)
        QVERIFY(MackieControlProtocol::channelToNote(ch) != 0xFF);
    for (quint32 ch = MACKIE_SELECT_OFFSET; ch < MACKIE_SELECT_OFFSET+8; ch++)
        QVERIFY(MackieControlProtocol::channelToNote(ch) != 0xFF);
}

void Midi_Test::inputProfile_encoderMovementTypes()
{
    quint32 ch=0; uchar val=0;
    // CW > 127, CCW < 127
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x01, &ch, &val));
    QVERIFY(val > 127);
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x41, &ch, &val));
    QVERIFY(val < 127);
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_JOG_WHEEL, 0x01, &ch, &val));
    QVERIFY(val > 127);
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_JOG_WHEEL, 0x41, &ch, &val));
    QVERIFY(val < 127);
}

/****************************************************************************
 * Integration tests: Fixture/Patch/VC compatibility
 ****************************************************************************/

void Midi_Test::integration_faderToFixtureChannel()
{
    quint32 ch=0; uchar val=0;
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|0, 0x00, 0x60, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_FADER_OFFSET);
    QCOMPARE(val, uchar(192));
    QVERIFY(val <= 255);

    // Simulate DMX write
    QByteArray dmx(512, 0);
    dmx[0] = val;
    QCOMPARE((uchar)dmx[0], uchar(192));

    // Verify feedback
    uchar fc=0,fd1=0,fd2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(ch, val, &fc, &fd1, &fd2));
    QCOMPARE(fc, uchar(MIDI_PITCH_WHEEL|0));
}

void Midi_Test::integration_buttonToFunctionTrigger()
{
    quint32 ch=0; uchar val=0;
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_F1, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_FUNCTION_OFFSET);
    QVERIFY(val > 0); // triggers function start

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_F1, 0x00, &ch, &val));
    QCOMPARE(val, uchar(0)); // stops function

    // All 8 F buttons as scene triggers
    for (int i=0;i<8;i++) {
        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_F1+i, 0x7F, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_FUNCTION_OFFSET+i));
    }

    // Verify LED feedback for F1
    uchar fc=0,fd1=0,fd2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_FUNCTION_OFFSET, 255, &fc, &fd1, &fd2));
    QCOMPARE(fc, uchar(MIDI_NOTE_ON)); QCOMPARE(fd1, uchar(MCU_NOTE_F1));
}

void Midi_Test::integration_vpotToLevelControl()
{
    quint32 ch=0; uchar val=0;
    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE, 0x03, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_VPOT_OFFSET);
    QCOMPARE(val, uchar(130));
    QVERIFY(val > 127); // increment

    QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_VPOT_SW_BASE, 0x7F, &ch, &val));
    QCOMPARE(ch, (quint32)MACKIE_VPOT_PUSH_OFFSET);

    uchar fc=0,fd1=0,fd2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_VPOT_LED_OFFSET, 128, &fc, &fd1, &fd2));
    QCOMPARE(fc, uchar(MIDI_CONTROL_CHANGE)); QCOMPARE(fd1, uchar(MCU_CC_VPOT_LED_BASE));
}

void Midi_Test::integration_feedbackFromFixtureToMCU()
{
    for (int i=0; i<8; i++) {
        uchar fv = (uchar)(i*32);
        uchar fc=0,fd1=0,fd2=0;
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_FADER_OFFSET+i, fv, &fc, &fd1, &fd2));
        QCOMPARE(fc, uchar(MIDI_PITCH_WHEEL|i));

        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_SOLO_OFFSET+i, fv>0?255:0, &fc, &fd1, &fd2));
        QCOMPARE(fc, uchar(MIDI_NOTE_ON)); QCOMPARE(fd1, uchar(MCU_NOTE_SOLO_BASE+i));

        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_VPOT_LED_OFFSET+i, fv, &fc, &fd1, &fd2));
        QCOMPARE(fc, uchar(MIDI_CONTROL_CHANGE)); QCOMPARE(fd1, uchar(MCU_CC_VPOT_LED_BASE+i));
    }

    uchar fc=0,fd1=0,fd2=0;
    QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_FADER_OFFSET+8, 200, &fc, &fd1, &fd2));
    QCOMPARE(fc, uchar(MIDI_PITCH_WHEEL|8));
}

void Midi_Test::integration_multipleChannelStripsMapping()
{
    for (int s=0; s<8; s++) {
        quint32 ch; uchar val;

        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_PITCH_WHEEL|s, 0x00, 0x7F, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_FADER_OFFSET+s));

        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_CONTROL_CHANGE, MCU_CC_VPOT_BASE+s, 0x01, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_VPOT_OFFSET+s));

        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_SOLO_BASE+s, 0x7F, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_SOLO_OFFSET+s));

        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_MUTE_BASE+s, 0x7F, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_MUTE_OFFSET+s));

        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_SELECT_BASE+s, 0x7F, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_SELECT_OFFSET+s));

        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, MCU_NOTE_REC_BASE+s, 0x7F, &ch, &val));
        QCOMPARE(ch, (quint32)(MACKIE_REC_OFFSET+s));

        uchar fc=0,fd1=0,fd2=0;
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_FADER_OFFSET+s, 128, &fc, &fd1, &fd2));
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_SOLO_OFFSET+s, 255, &fc, &fd1, &fd2));
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_MUTE_OFFSET+s, 255, &fc, &fd1, &fd2));
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_SELECT_OFFSET+s, 255, &fc, &fd1, &fd2));
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_REC_OFFSET+s, 255, &fc, &fd1, &fd2));
        QVERIFY(MackieControlProtocol::feedbackToMackie(MACKIE_VPOT_LED_OFFSET+s, 128, &fc, &fd1, &fd2));
    }

    // Transport for show control
    struct { uchar note; quint32 off; } tm[] = {
        {MCU_NOTE_PLAY, MACKIE_TRANSPORT_OFFSET+3}, {MCU_NOTE_STOP, MACKIE_TRANSPORT_OFFSET+2},
        {MCU_NOTE_REWIND, MACKIE_TRANSPORT_OFFSET+0}, {MCU_NOTE_FORWARD, MACKIE_TRANSPORT_OFFSET+1},
    };
    for (size_t i=0; i<sizeof(tm)/sizeof(tm[0]); i++) {
        quint32 ch; uchar val;
        QVERIFY(MackieControlProtocol::mackieToInput(MIDI_NOTE_ON, tm[i].note, 0x7F, &ch, &val));
        QCOMPARE(ch, tm[i].off);
    }
}

QTEST_MAIN(Midi_Test)
