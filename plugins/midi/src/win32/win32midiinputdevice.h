/*
  Q Light Controller
  win32midiinputdevice.h

  Copyright (c) Heikki Junnila

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

#ifndef WIN32MIDIINPUTDEVICE_H
#define WIN32MIDIINPUTDEVICE_H

#include <Windows.h>
#include <QObject>

#include "midiinputdevice.h"

class Win32MidiInputDevice final : public MidiInputDevice
{
    Q_OBJECT

public:
    Win32MidiInputDevice(const QVariant& uid, const QString& name, UINT id, QObject* parent = 0);
    ~Win32MidiInputDevice();

    bool open();
    void close();
    bool isOpen() const;

    bool processMBC(int type);

    // Allow callback to access m_handle for SysEx buffer re-add
    friend void CALLBACK MidiInProc(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

private:
    UINT m_id;
    HMIDIIN m_handle;
    QByteArray m_universe;
    uint m_mbc_counter;

    // SysEx receive buffer for Mackie Control handshake
    static const int SYSEX_BUFFER_SIZE = 256;
    char m_sysExBuffer[SYSEX_BUFFER_SIZE];
    MIDIHDR m_sysExHeader;
};

#endif
