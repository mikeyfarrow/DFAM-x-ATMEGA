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

#include "./midi_Events.h"
#include "./utilities.h"
#include "./MCP_4822.h"
#include "./debug_leds.h"
#include "./digital_outputs.h"

#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

uint16_t eep_idx = 0;

MIDI_CREATE_DEFAULT_INSTANCE();

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
}

/*
	serial_out - Output a byte to the USART0 port
*/
void serial_out(char ch)
{
	while ((UCSR0A & (1 << UDRE0 )) == 0) ;
	UDR0 = ch;
}

/*
	clear_eeprom - overwrites entire EEPROM memspace with 0
*/
void clear_eeprom()
{
	uint16_t addr = 0;
	while (addr <= E2END)
	{
		eeprom_write_byte((uint8_t*)addr, 0);
		addr += 1;
	}
}

void register_midi_callbacks()
{
	MIDI.setHandleNoteOn(cb_NoteOn);
	MIDI.setHandleStart(cb_Start);
	MIDI.setHandleStop(cb_Stop);
	MIDI.setHandleClock(cb_Clock);
}

int main()
{
	init_led_outputs();
	init_digital_outputs();
	init_midi_UART();
	init_DAC_SPI();
	
	register_midi_callbacks();
	
	clear_all_LEDs();
	clear_eeprom();
	
	send_pulse();
	send_trigger();

	int idx = 0;
	while (1)
	{
		if	(idx % 3 == 0) { status_led_green(); }
		else if (idx % 3 == 1) { status_led_red(); }
		else { status_led_off(); }
		
		MIDI.read(); // check for new message without blocking
		
		output_dac(0, 4095);
		output_dac(1, 4095);
		
		idx = (idx + 1) % 3;
	}
	return 0;
}