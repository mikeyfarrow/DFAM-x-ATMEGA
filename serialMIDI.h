/*!
*  @file       serialMIDI.h
*  Project     Arduino MIDI Library
*  @brief      MIDI Library for the Arduino - Platform
*  @license    MIT - Copyright (c) 2015 Francois Best
*  @author     lathoub, Francois Best
*  @date       22/03/20
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#pragma once

#include "midi_Namespace.h"
#include "midi_Defs.h"
#include "MIDI.h"

BEGIN_MIDI_NAMESPACE

// https://github.com/Juppi88/avr-serial

struct DefaultSerialSettings
{
	/*! Override the default MIDI baudrate to transmit over USB serial, to
	a decoding program such as Hairless MIDI (set baudrate to 115200)\n
	http://projectgus.github.io/hairless-midiserial/
	*/
	static const long BaudRate = 31250;
};

// TODO: Define SerialMidi transport class

class SerialMidiTransport
{
	private:
	MidiType mTxStatus;
	
	public:
	SerialMidiTransport(uint8_t cableNumber = 0)
	{
		// initialize fields ...
	};

	void begin()
	{
		
	};

	void end()
	{
		
	};

	// called by MIDI.hpp at line 129?
	bool beginTransmission(MidiType status)
	{
		return true;
	};

	void write(uint8_t byte)
	{
		
	};

	void endTransmission()
	{
		
	};

	uint8_t read()
	{
		return 0x00;
	};

	unsigned available()
	{
		return 0;
	};
};

END_MIDI_NAMESPACE

#define MIDI_CREATE_INSTANCE(CableNr, Name)  \
MIDI_NAMESPACE::SerialMidiTransport serial##Name(CableNr);\
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMidiTransport> Name((MIDI_NAMESPACE::SerialMidiTransport&)serial##Name);

#define MIDI_CREATE_CUSTOM_INSTANCE(CableNr, Name, Settings)  \
MIDI_NAMESPACE::SerialMidiTransport serial##Name(CableNr);\
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::usbMidiTransport, Settings> Name((MIDI_NAMESPACE::SerialMidiTransport&)serial##Name);

#define MIDI_CREATE_DEFAULT_INSTANCE()  \
MIDI_CREATE_INSTANCE(0, MIDI)

