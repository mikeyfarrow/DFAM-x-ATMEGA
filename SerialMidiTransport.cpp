/*
* SerialMidiTransport.cpp
*
* Created: 7/10/2024 1:01:42 PM
* Author: mikey
*/

#include <avr/interrupt.h>

#include "lib/midi_Defs.h"
#include "lib/midi_Namespace.h"

#include "./GPIO.h"
#include "SerialMidiTransport.h"

SMT::SerialMidiTransport()
{
	latest_serial_byte = MidiType::InvalidType;
	//uint8_t midi_rx_buffer[BUFFER_MAX_SIZE];
}

void SMT::begin() { };				/* nothing to do */
void SMT::end() { };				/* nothing to do */
void SMT::endTransmission() { };	/* nothing to do */
	
bool SMT::beginTransmission(MidiType status)
{
	return true;
}

	
void SMT::write(uint8_t msg)
{
	//cli();
	midi_tx_buffer.put(msg);
	//sei();
};


uint8_t SMT::read()
{
	//cli();
	uint8_t available = midi_rx_buffer.get(&latest_serial_byte);
	//sei();
	
	if (available)
	{
		return latest_serial_byte;
	}
	else
	{
		// the buffer must have been empty
		return MidiType::InvalidType;
	}
};

unsigned SMT::available()
{
	//cli();
	uint8_t result = midi_rx_buffer.ready();
	//sei();
	
	return result;
};

