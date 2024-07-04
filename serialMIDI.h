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

#include <avr/io.h>

#include "midi_Namespace.h"
#include "midi_Defs.h"
#include "MIDI.h"


#include <avr/eeprom.h>

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

class SerialMidiTransport
{
	private:
	uint16_t count;
	uint8_t is_ready;
	uint8_t latest_serial_byte;
	
	public:
	SerialMidiTransport(uint8_t cableNumber = 0)
	{
		count = 0;
		is_ready = false;
		latest_serial_byte = MidiType::InvalidType;
	};

	void begin() { /* nothing to do as long as UART has been initialized */ };
	void end() { /* nothing to do */ };
	bool beginTransmission(MidiType status) { return true; /* nothing to do */ };
	void endTransmission() { /* nothing to do */ };

	void write(uint8_t byte) {
		
	};

	uint8_t read()
	{
		if (!is_ready) return MidiType::InvalidType;
		
		is_ready = false; // only allow one read
		return latest_serial_byte;
	};

	unsigned available()
	{
		if ( !( UCSR0A & (1 << RXC0 )) ) return 0;
		
		latest_serial_byte = UDR0;
		eeprom_write_byte((uint8_t*)count, latest_serial_byte);
		count++;
		is_ready = true;
		return 1; // TODO: maybe include a buffer for incoming messages
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

