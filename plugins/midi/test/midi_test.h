/*
  Q Light Controller Plus
  midi_test.h

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

#ifndef MIDI_TEST_H
#define MIDI_TEST_H

#include <QObject>

class Midi_Test final : public QObject
{
    Q_OBJECT

private slots:
    void midiToInput();

    // Mackie Control Protocol: Input conversion
    void mackieToInput_faders();
    void mackieToInput_buttons();
    void mackieToInput_vpots();
    void mackieToInput_jogWheel();
    void mackieToInput_vuMeters();

    // Mackie Control Protocol: Feedback conversion
    void feedbackToMackie_faders();
    void feedbackToMackie_buttons();
    void feedbackToMackie_vpotLeds();
    void feedbackToMackie_vuMeters();
    void feedbackToMackie_7segment();

    // Mackie Control Protocol: Mapping consistency
    void noteToChannel_roundTrip();

    // Mackie Control Protocol: Round-trip tests
    void faderRoundTrip();
    void vuMeterRoundTrip();
    void buttonRoundTrip();

    // Mackie Control Protocol: VPot LED encoding
    void encodeVPotLed();

    // Mackie Control Protocol: Handshake algorithm
    void challengeResponseAlgorithm();

    // Mackie Control Protocol: Edge cases
    void mackieToInput_invalidMessages();
    void feedbackToMackie_unmappedChannels();
    void mackieToInput_vpotBoundary();
    void mackieToInput_vuOverRange();

    // Mackie Control Protocol: Channel layout consistency
    void channelLayoutNoOverlap();
    void allButtonNotesAreMapped();
    void globalViewSubButtonsMapping();

    // MackieControlHandler: Tests
    void handler_initialState();
    void handler_handshakeStateMachine();
    void handler_challengeResponse();
    void handler_sysExValidation();
    void handler_lcdOutput();
    void handler_clearLCD();
    void handler_7segOutput();
    void handler_vuMeterOutput();
    void handler_vuMeterClear();
    void handler_vpotLedOutput();
    void handler_nullOutputDevice();

    // Input profile: QXI channel verification
    void inputProfile_loadAndVerifyChannels();
    void inputProfile_channelTypes();
    void inputProfile_encoderMovementTypes();

    // Integration: Fixture/Patch/VC compatibility
    void integration_faderToFixtureChannel();
    void integration_buttonToFunctionTrigger();
    void integration_vpotToLevelControl();
    void integration_feedbackFromFixtureToMCU();
    void integration_multipleChannelStripsMapping();
};

#endif
