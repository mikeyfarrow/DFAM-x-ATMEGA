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
void init_timer1()
{
   // Set normal mode
   TCCR1A = 0; // WGM11 and WGM10 = 0
   TCCR1B = (1 << CS11) | (1 << CS10);
}

#define F_TIMER2 1000
void init_milli_counter_timer()
{
	// Configure timer 0 as fast PWM stopping at OCR0A (CTC should work as well)
	TCCR2A |= (1<<WGM00) | (1<<WGM01);

	// Select highest prescaler where 256 steps take more than 1ms for given F_CPU:
	// 1/F_CPU * 2^8 * Prescaler >= 1/1000 s => Prescaler >= F_CPU / 2^8 / 1000
	//        fast PWM     Prescaler = 64 for F_CPU = 16000000
	TCCR2B |= (1<<WGM02) | (1<<CS00) | (1<<CS01);

	// Select ticks after 1ms has passed:
	// 1/F_CPU * ticks * Prescaler = 1/1000s => ticks = F_CPU / Prescaler / 1000
	OCR2A = F_CPU / 64 / F_TIMER2 - 1; // -1 because it starts at zero and step from limit to zero counts as well

	// Enable interrupt routine ISR(TIMER0_COMPA_vect)
	TIMSK2 |= (1<<OCIE2A);
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
	UCSR0B |= RX_COMPLETE_INTERRUPT; // enable Rx interrupt
	
	UCSR0C = (3 << UCSZ00 ); // Set for async operation, no parity, 1 stop bit, 8 data bits
	
	DDRD |= _BV(PORTD1);
}

// Initialize the SPI as master
void init_DAC_SPI()
{
	// make the MOSI, SCK, and SS pins outputs
	SPI_DDR |= (1 << SPI_MOSI) | ( 1 << SPI_SCK) | (1 << SPI_SS);

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
}

void init_digital_inputs()
{
	DDR_MODE_SW &= ~_BV(DD_MODE_SW);
	DDR_SYNC_BTN &= ~_BV(DD_SYNC_BTN);
	DDR_LEARN_SW &= ~_BV(DD_LEARN_SW);
}

void init_led_outputs()
{
	DDRC |= _BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC4) | _BV(DDC5);
	DDRD |= _BV(DDD2) | _BV(DDD3);
}

void hardware_init()
{
	/* hardware/IO initialization */
	init_led_outputs();
	init_digital_outputs();
	init_digital_inputs();
	init_midi_UART();
	init_DAC_SPI();
	
	/* configure timers/counters and interrupts */
	init_pwm_output();
	init_milli_counter_timer();
	init_timer1();
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
