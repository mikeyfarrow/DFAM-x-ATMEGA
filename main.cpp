/*
 * FILE: main.cpp
 *
 * This file controls the event loop which is responsible for receiving and
 * transmitting MIDI messages. The event loop also handles analog IO via
 * ATMEGA's GPIO and the SPI communication with the DAC.
 */

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


/********************************************/
/*         MIDI Rx INTERRUPT                */
/********************************************/
ISR(USART_RX_vect) {
	uint8_t latest_byte = UDR0;
	
	if (serialMIDI.circ_buffer_put(latest_byte))
	{
		// nothing to do?
		// status2_green();
	}
	else
	{
		// TODO: Remove this!! this is just to detect if the buffer overflows
		status2_red();
		_delay_ms(1000);
	}
}

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
	MIDI.setHandleNoteOn(handleNoteOn);
	MIDI.setHandleStart(handleStart);
	MIDI.setHandleStop(handleStop);
	MIDI.setHandleClock(handleClock);
	MIDI.setHandleContinue(handleContinue);
	MIDI.setHandleControlChange(handleCC);
}

void running_status(uint16_t count)
{
	if	(count % 3 == 0) { status1_green(); }
	else if (count % 3 == 1) { status1_red(); }
	else{ status1_off(); }	
}

int main()
{
	init_led_outputs();				/* for debugging */
	init_digital_outputs();			/* advance clock out, trigger out x2,  ...velocities? */
	serialMIDI.init_midi_UART();	/* MIDI on the UART Tx/Rx pins */
	init_DAC_SPI();					/* for sending commands to the DAC */
	init_trig_timer();				
	
	register_midi_callbacks();
	clear_all_LEDs();
	
	uint16_t idx = 0;
	while (1)
	{
		running_status(idx);

		MIDI.read(); /* check for new message without blocking */
		
		idx++;
	}
	return 0;
}