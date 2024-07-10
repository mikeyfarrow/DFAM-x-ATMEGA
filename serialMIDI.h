/*
 * FILE: serialMIDI.h
 *
 * This file provides an implementation of the Transport template (from arduino MIDI library)
 * that is configured for the ATMEGA328's UART pins.
 */
#pragma once

#include <avr/io.h>
#include <avr/eeprom.h>

#include "lib/midi_Namespace.h"
#include "lib/midi_Defs.h"
#include "lib/MIDI.h"


#define CPU_HZ 16000000UL
#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((CPU_HZ / (USART_BAUD_RATE * 16UL))) - 1)
#define BUFFER_MAX_SIZE 100

BEGIN_MIDI_NAMESPACE


struct DefaultSerialSettings
{
	static const long BaudRate = USART_BAUD_RATE;
};

class SerialMidiTransport
{
	private:
	uint8_t latest_serial_byte = MidiType::InvalidType;
	uint8_t midi_rx_buffer[BUFFER_MAX_SIZE];
	uint8_t write_idx = 0;
	uint8_t read_idx = 0;
	
	/*
		circ_buffer_get - if there is a value in the buffer, it is saved to
			the address pointed to by val_ptr and returns 1. If there is no
			data in the buffer, then returns 0.
	*/
	uint8_t circ_buffer_get(uint8_t* val_ptr)
	{
		if (read_idx == write_idx)
		{
			return 0; // buffer is empty
		}
		
		*val_ptr = midi_rx_buffer[read_idx];
		read_idx = (read_idx + 1) % BUFFER_MAX_SIZE;
		return 1;
	};
	
	public:
	SerialMidiTransport() { };
		
	/*	circ_buffer_put - adds the item to the buffer and returns 1. If the add fails
			because the buffer is full, then we do not add the item and
			return 0
	*/
	uint8_t circ_buffer_put(uint8_t item)
	{
		if ((write_idx + 1) % BUFFER_MAX_SIZE == read_idx)
		{
			// buffer is full, avoid overflow
			return 0;
		}
		
		midi_rx_buffer[write_idx] = item;
		write_idx = (write_idx + 1) % BUFFER_MAX_SIZE;
		return 1;
	};
	
	

	void begin() { };  /* UART already initialized*/          /* nothing to do */
	void end() { };											  /* nothing to do */
	bool beginTransmission(MidiType status) { return true; }; /* nothing to do */
	void endTransmission() { };								  /* nothing to do */

	void write(uint8_t byte) {
		/* TODO: implement serial write (for MIDI Tx) */
	};

	/*
		SerialMidi
	*/
	uint8_t read()
	{
		if (circ_buffer_get(&latest_serial_byte))
		{
			return latest_serial_byte;
		}
		else
		{
			// the buffer must have been empty
			return MidiType::InvalidType;
		}
	};

	unsigned available()
	{
		return read_idx != write_idx; // return the difference maybe??
	};
};

END_MIDI_NAMESPACE
