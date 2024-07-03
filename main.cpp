// See:
//		https://www.arnabkumardas.com/arduino-tutorial/usart-programming/
//		https://ece-classes.usc.edu/ee459/library/documents/Serial_Port.pdf

#define F_CPU 16000000UL // Defining the CPU Frequency

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "./midi_messages.h"
#include "./utilities.h"
#include "./debug_leds.h"

MIDI_CREATE_DEFAULT_INSTANCE()

// LED IO
#define LED_PORT PORTD
#define LED_CLOCK PORTD4
#define LED_START PORTD3
#define LED_STOP PORTD2
#define LED_OTHER PORTD5

#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

/*
	serial_init - Initialize the USART port
	
		MIDI spec:
		- no parity bit
		- baud rate is 31.25 Kbaud
		- 1 start bit
		- 8 data bits
		- 1 stop bit
		
		transer rate for the 10 total bytes is 320 microseconds per serial byte
*/
void serial_init() {
	UBRR0H = BAUD_RATE_BYTES >> 8; // set the baud rate registers
	UBRR0L = BAUD_RATE_BYTES;
	
	//UCSR0B |= (1 << TXEN0 ); // Turn on transmitter
	UCSR0B |= (1 << RXEN0 ); // Turn on receiver
	UCSR0C = (3 << UCSZ00 ); // Set for async . operation , no parity , one stop bit , 8 data bits
}

/*
serial_in - Read a byte from the USART0 and return it
*/
char serial_in()
{
	while ( !( UCSR0A & (1 << RXC0 )) ); // wait until there's a message
	return UDR0;
}

/*
serial_out - Output a byte to the USART0 port
*/
void serial_out(char ch)
{
	while ((UCSR0A & (1 << UDRE0 )) == 0) ;
	UDR0 = ch;
}

void handle_midi_message(char midi_byte)
{
	switch (midi_byte)
	{
		case Clock: /*********************** CLOCK ***********************/
		set_bit(LED_PORT, LED_CLOCK);
		clear_bit(LED_PORT, LED_STOP);
		clear_bit(LED_PORT, LED_START);
		clear_bit(LED_PORT, LED_OTHER);
		break;
				
		case Start: /*********************** START ***********************/
		set_bit(LED_PORT, LED_START);
		clear_bit(LED_PORT, LED_STOP);
		clear_bit(LED_PORT, LED_CLOCK);
		clear_bit(LED_PORT, LED_OTHER);
		break;
				
		case Stop: /*********************** STOP ************************/
		set_bit(LED_PORT, LED_STOP);
		clear_bit(LED_PORT, LED_START);
		clear_bit(LED_PORT, LED_CLOCK);
		clear_bit(LED_PORT, LED_OTHER);
		break;
				
		default: /************************* N/A *************************/
		set_bit(LED_PORT, LED_OTHER);
		clear_bit(LED_PORT, LED_START);
		clear_bit(LED_PORT, LED_CLOCK);
		clear_bit(LED_PORT, LED_STOP);
		break;
	}
}

// PORTx for writing outputs
// PINx is read only
int main()
{
	//char messages[4] = { Clock, Start, Stop, Continue };
	int idx = 0;
	int num_messages = 4;
	
	// INTIALIZE GPIOs
	init_led_io();
	clear_all_LEDs();
	serial_init();
	
	while (1)
	{
		//if (idx % 3 == 0) status_led_green();
		//else if (idx % 3 == 1) status_led_red();
		//else status_led_off();
		
		//serial_out(messages[idx]);
		
		clear_bank_a();
		clear_bank_b();
		set_bit(BANK_A_PORT, BANK_A_LEDS[idx]);
		set_bit(BANK_B_PORT, BANK_B_LEDS[idx]);

		char midi_byte = serial_in();
		handle_midi_message(midi_byte);
		
		idx = (idx + 1) % num_messages;
	}
	return 0;
}
