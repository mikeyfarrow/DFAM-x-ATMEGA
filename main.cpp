// See:
//		https://www.arnabkumardas.com/arduino-tutorial/usart-programming/
//		https://ece-classes.usc.edu/ee459/library/documents/Serial_Port.pdf

#define F_CPU 16000000UL // Defining the CPU Frequency

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/eeprom.h>

#include "./MIDI.h"
#include "./midi_Defs.h"
#include "./midi_Events.h"
#include "./utilities.h"
#include "./debug_leds.h"

#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

uint16_t eep_idx = 0;

MIDI_CREATE_DEFAULT_INSTANCE();

/*
	serial_init - Initialize the USART port
		MIDI spec: no parity bit, 1 start bit, 8 data bits, 1 stop bit, baud=31250
*/
void serial_init()
{	
	UBRR0H = BAUD_RATE_BYTES >> 8; // baud rate is uint16_t so it takes up two registers
	UBRR0L = BAUD_RATE_BYTES;
	
	UCSR0B |= (1 << TXEN0 ); // Turn on transmitter
	UCSR0B |= (1 << RXEN0 ); // Turn on receiver
	UCSR0C = (3 << UCSZ00 ); // Set for async operation, no parity, 1 stop bit, 8 data bits
}

/*
	serial_in - Read a byte from the USART0 and return it
*/
char serial_in()
{
	bank_A(0);
	while ( !( UCSR0A & (1 << RXC0 )) ) return 0; // wait until there's a message
	
	bank_A(1);
	char ch = UDR0;
	
	eeprom_write_byte((uint8_t*) eep_idx, 0x22);
	eep_idx++;
	
	eeprom_write_byte((uint8_t*) eep_idx, ch);
	bank_A(2);
	eep_idx++;
	
	return ch;
}

/*
serial_out - Output a byte to the USART0 port
*/
void serial_out(char ch)
{
	while ((UCSR0A & (1 << UDRE0 )) == 0) ;
	
	UDR0 = ch;

}

void clear_eeprom()
{
	uint16_t addr = 0;
	while (addr <= E2END)
	{
		eeprom_write_byte((uint8_t*)addr, 0);
		addr += 1;
	}
}

void handle_midi_message(char midi_byte)
{
	bank_B_off();
	switch (midi_byte)
	{
		case MIDI_NAMESPACE::Clock: /********** CLOCK ***********************/
			bank_B(0);
			break;
				
		case MIDI_NAMESPACE::Start: /********** START ***********************/
			bank_B(1);
			break;
				
		case MIDI_NAMESPACE::Stop:  /********** STOP ************************/
			bank_B(2);
			break;
				
		default:				    /********** N/A *************************/
			bank_B(3);
			break;
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
	char messages[4] = { MIDI_NAMESPACE::Start, MIDI_NAMESPACE::Start, MIDI_NAMESPACE::Clock, MIDI_NAMESPACE::NoteOn };
	int num_messages = 4;
	init_led_io();
	serial_init();
	register_midi_callbacks();
	
	clear_all_LEDs();
	clear_eeprom();

	int idx = 0;
	while (1)
	{
		if	(idx % 3 == 0) status_led_green();
		else if (idx % 3 == 1) status_led_red();
		else status_led_off();
		
		MIDI.read();
		
		serial_out(messages[idx]);
		
		idx = (idx + 1) % 3;
	}
	return 0;
}