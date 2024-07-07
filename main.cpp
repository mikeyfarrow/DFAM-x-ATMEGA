/*
 * FILE: main.cpp
 *
 * This file controls the event loop which is responsible for receiving and
 * transmitting MIDI messages. The event loop also handles analog IO via
 * ATMEGA's GPIO and the SPI communication with the DAC.
 */

// See:
//		https://www.arnabkumardas.com/arduino-tutorial/usart-programming/
//		https://ece-classes.usc.edu/ee459/library/documents/Serial_Port.pdf

#define F_CPU 16000000UL // 16 MHz (required by delay.h)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "lib/MIDI.h"
#include "lib/midi_Defs.h"

#include "./debug_leds.h"
#include "./midi_Events.h"
#include "./utilities.h"
#include "./MCP_4822.h"
#include "./digital_outputs.h"
#include "./EEPROM_Writer.h"

typedef	MIDI_NAMESPACE::SerialMidiTransport SerialTransport;
typedef MIDI_NAMESPACE::MidiInterface<SerialTransport> MidiInterface;

/* for debugging */
EEPROM_Writer ew = EEPROM_Writer();

/* create MIDI instances */
SerialTransport serialMIDI;
MidiInterface	MIDI((SerialTransport&) serialMIDI);

/*
	serial_out - Output a byte to the USART0 port
*/
void serial_out(char ch)
{
	while ((UCSR0A & (1 << UDRE0 )) == 0) ;
	UDR0 = ch;
}

void register_midi_callbacks()
{
	MIDI.setHandleNoteOn(cb_NoteOn);
	MIDI.setHandleStart(cb_Start);
	MIDI.setHandleStop(cb_Stop);
	MIDI.setHandleClock(cb_Clock);
	MIDI.setHandleContinue(cb_Continue);
	MIDI.setHandleControlChange(cb_ControlChange);
}

int main()
{
	init_led_outputs();
	init_digital_outputs();
	serialMIDI.init_midi_UART();
	init_DAC_SPI();
	init_trig_timer();

	register_midi_callbacks();
	clear_all_LEDs();
	
	int idx = 0;
	while (1)
	{
		
		if	(idx % 3 == 0) { status1_green(); }
		else if (idx % 3 == 1) { status1_red(); }
		else { status1_off(); }
		
		MIDI.read(); // check for new message without blocking
		
		idx = (idx + 1) % 3;
	}
	return 0;
}