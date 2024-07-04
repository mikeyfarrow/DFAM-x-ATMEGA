/*
 * FILE: serialMIDI.h
 *
 * This file provides an implementation of the Transport template (from arduino MIDI library)
 * that is configured for the ATMEGA328's UART pins.
 */
#pragma once

#include <avr/io.h>
#include <avr/eeprom.h>

#include "midi_Namespace.h"
#include "midi_Defs.h"
#include "MIDI.h"

BEGIN_MIDI_NAMESPACE

// https://github.com/Juppi88/avr-serial

struct DefaultSerialSettings
{
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

	void begin() { };  /* UART already initialized*/          /* nothing to do */
	void end() { };											  /* nothing to do */
	bool beginTransmission(MidiType status) { return true; }; /* nothing to do */
	void endTransmission() { };								  /* nothing to do */

	void write(uint8_t byte) {
		/* TODO: implement serial write (for MIDI Tx) */
	};

	uint8_t read()
	{
		if (!is_ready) 
		{
			return MidiType::InvalidType;
		}
		else
		{
			is_ready = false; // only allow one read
			return latest_serial_byte;
		}
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

