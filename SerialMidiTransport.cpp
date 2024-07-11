/*
* SerialMidiTransport.cpp
*
* Created: 7/10/2024 1:01:42 PM
* Author: mikey
*/

#include "lib/midi_Defs.h"
#include "lib/midi_Namespace.h"

#include "SerialMidiTransport.h"

SMT::SerialMidiTransport()
{
	latest_serial_byte = MidiType::InvalidType;
	//uint8_t midi_rx_buffer[BUFFER_MAX_SIZE];
}

void SMT::begin() { };  /* UART already initialized */			/* nothing to do */
void SMT::end() { };											/* nothing to do */
bool SMT::beginTransmission(MidiType status) { return true; };	/* nothing to do */
void SMT::endTransmission() { };								/* nothing to do */
void SMT::write(uint8_t byte) { };		         /* TODO: implement for MIDI Tx? */


uint8_t SMT::read()
{
	if (midi_rx_buffer.get(&latest_serial_byte))
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
	return midi_rx_buffer.ready();
};

