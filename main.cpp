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

#define F_CPU 16000000UL // Defining the CPU Frequency

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/eeprom.h>

#include "lib/MIDI.h"
#include "lib/midi_Defs.h"

#include "./debug_leds.h"
//#include "./rotary_encoder.h"
#include "./midi_Events.h"
#include "./utilities.h"
#include "./MCP_4822.h"
#include "./digital_outputs.h"
#include "./EEPROM_Writer.h"

#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

uint16_t eep_idx = 0;

MIDI_CREATE_DEFAULT_INSTANCE();
EEPROM_Writer ew = EEPROM_Writer();

/*
	serial_init - Initialize the USART port
		MIDI spec: no parity bit, 1 start bit, 8 data bits, 1 stop bit, baud=31250
*/
void init_midi_UART()
{	
	UBRR0H = BAUD_RATE_BYTES >> 8; // baud rate is uint16_t so it takes up two registers
	UBRR0L = BAUD_RATE_BYTES;
	
	UCSR0B |= (1 << TXEN0 ); // Turn on transmitter
	UCSR0B |= (1 << RXEN0 ); // Turn on receiver
	UCSR0C = (3 << UCSZ00 ); // Set for async operation, no parity, 1 stop bit, 8 data bits
	
	DDRD |= _BV(PORTD1);
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
	init_midi_UART();
	init_DAC_SPI();
	init_trig_timer();

	register_midi_callbacks();
	
	clear_all_LEDs();
	
	advance_clock();
	trigger();
	
	
	
	serial_out(0xDD);
	serial_out(0xBB);

	int idx = 0;
	while (1)
	{
		
		if	(idx % 3 == 0) { status_led_green(); }
		else if (idx % 3 == 1) { status_led_red(); }
		else { status_led_off(); }
		
		//serial_out(0xBE);
		//serial_out(0xEF);
		MIDI.read(); // check for new message without blocking
		
		
		idx = (idx + 1) % 3;
	}
	return 0;
}