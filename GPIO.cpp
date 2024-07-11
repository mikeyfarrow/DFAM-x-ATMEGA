/*
 * GPIO.cpp
 *
 * Created: 7/10/2024 4:34:01 PM
 *  Author: mikey
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "GPIO.h"


uint8_t LED_BANK[3] = { LED1, LED2, LED3 };		


/***************************************************/
/*	TIMER 0 - fast PWM with outputs on PD6 and PD3 */
void init_pwm_output()
{
	// frequency = 8 khz
	// see https://avr-guide.github.io/pwm-on-the-atmega328/
	DDRD |= (1 << DDD6) | (1<<DDD5);		// PD6 & PD5 is now an output
	TCCR0A |= (1 << COM0A1) | (1<<COM0B1);	// set none-inverted mode for both output compares
	TCCR0A |= (1 << WGM01) | (1 << WGM00);	// set fast PWM Mode
	OCR0A = 0x00;							// pwm out #1 duty cycle 0
	OCR0B = 0x00;							// pwm out #2 duty cycle 0
	TCCR0B |= (1 << CS00);					// set prescaler to none and start PWM (f=62500Hz)
}

/*******************************************/
/* TIMER 1 - Interrupt every 1 millisecond */
void init_timer_interrupt()
{
	cli(); // disable interrupts globally
    
	TCCR1A = 0;		// set entire TCCR1A register to 0
	TCCR1B = 0;		// same for TCCR1B
	TCNT1  = 0;		// initialize counter value to 0
	OCR1A = 15999;										// 1000 Hz i.e. 1 ms
	TCCR1B |= (1 << WGM12);								// turn on CTC mode
	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);	// clock div. = 1
	TIMSK1 |= (1 << OCIE1A);							// enable timer compare interrupt

	// see also: https://avr-guide.github.io/assets/docs/Atmel-2542-Using-the-AVR-High-speed-PWM_ApplicationNote_AVR131.pdf
	//			 http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
	
	sei(); // enable interrupts globally
}

/*	init_midi_UART - Initialize the USART port
		MIDI spec: no parity bit, 1 start bit, 8 data bits, 1 stop bit, baud=31250	
*/
void init_midi_UART()
{	
	UBRR0H = BAUD_RATE_BYTES >> 8; // baud rate is uint16_t so it takes up two registers
	UBRR0L = BAUD_RATE_BYTES;
	
	UCSR0B |= (1 << TXEN0 ); // enable transmitter
	UCSR0B |= (1 << RXEN0 ); // enable receiver
	UCSR0B |= (1 << RXCIE0); // enable Rx interrupt
	UCSR0C = (3 << UCSZ00 ); // Set for async operation, no parity, 1 stop bit, 8 data bits
	
	DDRD |= _BV(PORTD1);
}

// Initialize the SPI as master
void init_DAC_SPI()
{
	// make the MOSI, SCK, and SS pins outputs
	SPI_DDR |= ( 1 << SPI_MOSI ) | ( 1 << SPI_SCK ) | ( 1 << SPI_SS );

	// TODO: no it is not, not used by DAC
	// make sure the MISO pin is input
	SPI_DDR &= ~( 1 << SPI_MISO );

	// set up the SPI module: SPI enabled, MSB first, master mode,
	//  clock polarity and phase = 0, F_osc/16
	SPI_SPCR = ( 1 << SPI_SPE ) | ( 1 << SPI_MSTR );// | ( 1 << SPI_SPR0 );
	SPI_SPSR = 1;     // set double SPI speed for F_osc/2
}

void init_digital_outputs()
{
	DDR_ADV |= (1<<DD_ADV);
	
	DDR_TRIG |= (1<<DD_TRIGA);
	DDR_VEL |= (1<<DD_VELA);
	
	DDR_TRIG |= (1<<DD_TRIGB);
	DDR_VEL |= (1<<DD_VELB);
	
	// TODO: pull downs? ups?
}

void init_led_outputs()
{
	DDRC |= _BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC4) | _BV(DDC5);
	DDRD |= _BV(DDD2) | _BV(DDD3);
}


void set_all_LEDs()
{
	status1_green();
	status2_green();
	set_bank();
}

void clear_all_LEDs()
{
	status1_off();
	status2_off();
	clear_bank();
}

void bank(uint8_t i) {
	set_bit(LED_BANK_PORT, LED_BANK[i]);
}

void bank_off(uint8_t i) {
	clear_bit(LED_BANK_PORT, LED_BANK[i]);
}
