/*
* SerialMidiTransport.h
*
* Created: 7/10/2024 1:01:42 PM
* Author: mikey
*/


#ifndef __SERIALMIDITRANSPORT_H__
#define __SERIALMIDITRANSPORT_H__

#include <avr/io.h>
#include <avr/eeprom.h>

#include "CircularBuffer.h"
#include "lib/midi_Namespace.h"
#include "lib/midi_Defs.h"
#include "lib/MIDI.h"

#define BUFFER_MAX_SIZE 100

#define SMT MIDI_NAMESPACE::SerialMidiTransport

BEGIN_MIDI_NAMESPACE
class SerialMidiTransport
{
/***** FIELDS *****/
public:
	CircularBuffer midi_rx_buffer;
	
private:
	uint8_t latest_serial_byte;

/***** METHODS *****/
public:
	SerialMidiTransport();
	void begin();
	void end();
	bool beginTransmission(MIDI_NAMESPACE::MidiType status);
	void endTransmission();
	void write(uint8_t byte);
	uint8_t read();
	unsigned available();
};

END_MIDI_NAMESPACE

#endif //__SERIALMIDITRANSPORT_H__
