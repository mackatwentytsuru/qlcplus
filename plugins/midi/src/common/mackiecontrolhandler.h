/*
  Q Light Controller Plus
  mackiecontrolhandler.h

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

#ifndef MACKIECONTROLHANDLER_H
#define MACKIECONTROLHANDLER_H

#include <QByteArray>
#include <QObject>
#include <QString>

#include "mackiecontrolprotocol.h"

class MidiOutputDevice;

class MackieControlHandler : public QObject
{
    Q_OBJECT

public:
    enum HandshakeState
    {
        Idle,
        WaitingForChallenge,
        WaitingForConfirm,
        Connected
    };

    MackieControlHandler(QObject* parent = nullptr);
    ~MackieControlHandler();

    /************************************************************************
     * Device configuration
     ************************************************************************/
    void setDeviceId(uchar deviceId);
    uchar deviceId() const;

    void setOutputDevice(MidiOutputDevice* device);
    MidiOutputDevice* outputDevice() const;

    /************************************************************************
     * Handshake
     ************************************************************************/
    void initiateHandshake();
    void handleSysEx(const QByteArray& data);
    HandshakeState handshakeState() const;

    static void computeChallengeResponse(const uchar challenge[4], uchar response[4]);

    /************************************************************************
     * LCD Display
     ************************************************************************/
    void updateLCD(int line, int channelIndex, const QString& text);
    void clearLCD();

    /************************************************************************
     * 7-Segment Display
     ************************************************************************/
    void update7Segment(const QString& text);

    /************************************************************************
     * VU Meters
     ************************************************************************/
    void updateVUMeter(int channel, uchar level);
    void clearVUMeters();

    /************************************************************************
     * VPot LED Rings
     ************************************************************************/
    void updateVPotLed(int vpot, uchar value, uchar mode = MCU_VPOT_MODE_SINGLE);

signals:
    void handshakeCompleted();
    void handshakeFailed();

private:
    void sendSysEx(const QByteArray& data);

    uchar m_deviceId;
    MidiOutputDevice* m_outputDevice;
    HandshakeState m_handshakeState;
    QByteArray m_serialNumber;
};

#endif // MACKIECONTROLHANDLER_H
