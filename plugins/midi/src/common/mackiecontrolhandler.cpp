/*
  Q Light Controller Plus
  mackiecontrolhandler.cpp

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

#include <QDebug>

#include "mackiecontrolhandler.h"
#include "midioutputdevice.h"
#include "midiprotocol.h"

MackieControlHandler::MackieControlHandler(QObject* parent)
    : QObject(parent)
    , m_deviceId(MCU_DEVICE_ID_MCU)
    , m_outputDevice(nullptr)
    , m_handshakeState(Idle)
{
}

MackieControlHandler::~MackieControlHandler()
{
}

/*****************************************************************************
 * Device configuration
 *****************************************************************************/

void MackieControlHandler::setDeviceId(uchar deviceId)
{
    m_deviceId = deviceId;
}

uchar MackieControlHandler::deviceId() const
{
    return m_deviceId;
}

void MackieControlHandler::setOutputDevice(MidiOutputDevice* device)
{
    m_outputDevice = device;
}

MidiOutputDevice* MackieControlHandler::outputDevice() const
{
    return m_outputDevice;
}

/*****************************************************************************
 * Handshake
 *****************************************************************************/

void MackieControlHandler::initiateHandshake()
{
    qDebug() << "[MCU] Initiating handshake with device ID:" << m_deviceId;

    // Send Device Query: F0 00 00 66 [deviceId] 00 F7
    QByteArray query;
    query.append((char)0xF0);
    query.append((char)MCU_SYSEX_MANUFACTURER_1);
    query.append((char)MCU_SYSEX_MANUFACTURER_2);
    query.append((char)MCU_SYSEX_MANUFACTURER_3);
    query.append((char)m_deviceId);
    query.append((char)MCU_SYSEX_CMD_QUERY);
    query.append((char)0xF7);

    m_handshakeState = WaitingForChallenge;
    sendSysEx(query);
}

void MackieControlHandler::handleSysEx(const QByteArray& data)
{
    // Minimum SysEx: F0 00 00 66 [deviceId] [cmd] ... F7
    if (data.size() < 7)
        return;

    // Verify Mackie SysEx header
    if ((uchar)data[1] != MCU_SYSEX_MANUFACTURER_1 ||
        (uchar)data[2] != MCU_SYSEX_MANUFACTURER_2 ||
        (uchar)data[3] != MCU_SYSEX_MANUFACTURER_3)
        return;

    uchar devId = (uchar)data[4];
    uchar cmd = (uchar)data[5];

    // Accept messages for our device ID
    if (devId != m_deviceId)
        return;

    switch (cmd)
    {
        case MCU_SYSEX_CMD_CHALLENGE:
        {
            // Host Connection Query: F0 00 00 66 [id] 01 [serial x7] [challenge x4] F7
            if (data.size() < 18)
            {
                qDebug() << "[MCU] Invalid challenge message size:" << data.size();
                return;
            }

            // Extract serial number (7 bytes, starting at index 6)
            m_serialNumber = data.mid(6, 7);

            // Extract challenge (4 bytes, starting at index 13)
            uchar challenge[4];
            for (int i = 0; i < 4; i++)
                challenge[i] = (uchar)data[13 + i];

            // Compute response
            uchar response[4];
            computeChallengeResponse(challenge, response);

            // Send Host Connection Reply: F0 00 00 66 [id] 02 [serial x7] [response x4] F7
            QByteArray reply;
            reply.append((char)0xF0);
            reply.append((char)MCU_SYSEX_MANUFACTURER_1);
            reply.append((char)MCU_SYSEX_MANUFACTURER_2);
            reply.append((char)MCU_SYSEX_MANUFACTURER_3);
            reply.append((char)m_deviceId);
            reply.append((char)MCU_SYSEX_CMD_RESPONSE);
            reply.append(m_serialNumber);
            for (int i = 0; i < 4; i++)
                reply.append((char)response[i]);
            reply.append((char)0xF7);

            m_handshakeState = WaitingForConfirm;
            sendSysEx(reply);

            qDebug() << "[MCU] Challenge received, response sent";
            break;
        }

        case MCU_SYSEX_CMD_CONFIRM:
        {
            // Host Connection Confirmation
            m_handshakeState = Connected;
            qDebug() << "[MCU] Handshake completed successfully";
            emit handshakeCompleted();
            break;
        }

        default:
            qDebug() << "[MCU] Unhandled SysEx command:" << cmd;
            break;
    }
}

MackieControlHandler::HandshakeState MackieControlHandler::handshakeState() const
{
    return m_handshakeState;
}

void MackieControlHandler::computeChallengeResponse(const uchar challenge[4], uchar response[4])
{
    // Mackie Control challenge/response algorithm
    response[0] = 0x7F & (challenge[0] + (challenge[1] ^ 0x0A) - challenge[3]);
    response[1] = 0x7F & ((challenge[2] >> 4) ^ (challenge[0] + challenge[3]));
    response[2] = 0x7F & ((challenge[3] - (challenge[2] << 2)) ^ (challenge[0] | challenge[1]));
    response[3] = 0x7F & (challenge[1] - challenge[2] + (0xF0 ^ (challenge[3] << 4)));
}

/*****************************************************************************
 * LCD Display
 *****************************************************************************/

void MackieControlHandler::updateLCD(int line, int channelIndex, const QString& text)
{
    if (m_outputDevice == nullptr)
        return;

    // Each channel gets 7 characters on the LCD
    // Line 0: offsets 0x00-0x37 (56 chars)
    // Line 1: offsets 0x38-0x6F (56 chars)
    uchar offset = (line * 0x38) + (channelIndex * 7);

    // Pad/truncate text to 7 characters
    QString padded = text.leftJustified(7, ' ').left(7);

    QByteArray sysex;
    sysex.append((char)0xF0);
    sysex.append((char)MCU_SYSEX_MANUFACTURER_1);
    sysex.append((char)MCU_SYSEX_MANUFACTURER_2);
    sysex.append((char)MCU_SYSEX_MANUFACTURER_3);
    sysex.append((char)m_deviceId);
    sysex.append((char)MCU_SYSEX_CMD_LCD);
    sysex.append((char)offset);
    sysex.append(padded.toLatin1());
    sysex.append((char)0xF7);

    sendSysEx(sysex);
}

void MackieControlHandler::clearLCD()
{
    if (m_outputDevice == nullptr)
        return;

    // Clear both lines with spaces
    QString spaces(56, ' ');

    // Line 1
    QByteArray sysex1;
    sysex1.append((char)0xF0);
    sysex1.append((char)MCU_SYSEX_MANUFACTURER_1);
    sysex1.append((char)MCU_SYSEX_MANUFACTURER_2);
    sysex1.append((char)MCU_SYSEX_MANUFACTURER_3);
    sysex1.append((char)m_deviceId);
    sysex1.append((char)MCU_SYSEX_CMD_LCD);
    sysex1.append((char)0x00);
    sysex1.append(spaces.toLatin1());
    sysex1.append((char)0xF7);
    sendSysEx(sysex1);

    // Line 2
    QByteArray sysex2;
    sysex2.append((char)0xF0);
    sysex2.append((char)MCU_SYSEX_MANUFACTURER_1);
    sysex2.append((char)MCU_SYSEX_MANUFACTURER_2);
    sysex2.append((char)MCU_SYSEX_MANUFACTURER_3);
    sysex2.append((char)m_deviceId);
    sysex2.append((char)MCU_SYSEX_CMD_LCD);
    sysex2.append((char)0x38);
    sysex2.append(spaces.toLatin1());
    sysex2.append((char)0xF7);
    sendSysEx(sysex2);
}

/*****************************************************************************
 * 7-Segment Display
 *****************************************************************************/

void MackieControlHandler::update7Segment(const QString& text)
{
    if (m_outputDevice == nullptr)
        return;

    // 7-segment display has 12 digits (CC 64-75), right to left
    // We send each character as a CC message
    QString padded = text.rightJustified(12, ' ').right(12);

    for (int i = 0; i < 12; i++)
    {
        char ch = padded.at(11 - i).toLatin1();
        // Only send printable ASCII in range 0x30-0x5F
        uchar displayChar = (uchar)ch;
        if (displayChar < 0x20)
            displayChar = 0x20;  // space

        uchar cmd = MIDI_CONTROL_CHANGE;  // Channel 0
        uchar cc = MCU_CC_7SEG_BASE + i;
        m_outputDevice->writeFeedback(cmd, cc, displayChar & 0x7F);
    }
}

/*****************************************************************************
 * VU Meters
 *****************************************************************************/

void MackieControlHandler::updateVUMeter(int channel, uchar level)
{
    if (m_outputDevice == nullptr || channel < 0 || channel > 7)
        return;

    // Scale 0-255 to 0-12
    uchar vuLevel = (level * 12) / 255;

    // Channel Pressure: high nibble = channel, low nibble = level
    uchar data = ((uchar)channel << 4) | (vuLevel & 0x0F);
    m_outputDevice->writeFeedback(MIDI_CHANNEL_AFTERTOUCH, data, 0);
}

void MackieControlHandler::clearVUMeters()
{
    for (int i = 0; i < 8; i++)
        updateVUMeter(i, 0);
}

/*****************************************************************************
 * VPot LED Rings
 *****************************************************************************/

void MackieControlHandler::updateVPotLed(int vpot, uchar value, uchar mode)
{
    if (m_outputDevice == nullptr || vpot < 0 || vpot > 7)
        return;

    // Scale 0-255 to 0-11 (LED ring position)
    uchar position = (value * 11) / 255;
    uchar encoded = MackieControlProtocol::encodeVPotLed(position, mode);

    m_outputDevice->writeFeedback(MIDI_CONTROL_CHANGE,
                                   MCU_CC_VPOT_LED_BASE + vpot,
                                   encoded);
}

/*****************************************************************************
 * Private
 *****************************************************************************/

void MackieControlHandler::sendSysEx(const QByteArray& data)
{
    if (m_outputDevice != nullptr)
        m_outputDevice->writeSysEx(data);
}
