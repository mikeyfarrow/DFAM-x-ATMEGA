/*
 * FILE: main.cpp
 *
 * This file controls the event loop which is responsible for receiving and
 * transmitting MIDI messages. The event loop also handles analog IO via
 * ATMEGA's GPIO and the SPI communication with the DAC.
 */

#define F_CPU 16000000UL // 16 MHz (required by delay.h)
#define CPU_HZ 16000000UL
#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((CPU_HZ / (USART_BAUD_RATE * 16UL))) - 1)
#define BUFFER_MAX_SIZE 100

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/eeprom.h>
#include <util/delay.h>


#include "GPIO.h"
#include "MidiController.h"
#include "utilities.h"

/** Instantiate the controller **/
MidiController mctl;

// Event handlers
void register_midi_events();
void handleCC(byte channel, byte cc_num, byte cc_val);
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleStart();
void handleStop();
void handleClock();
void handleContinue();


void running_status(uint16_t count)
{
	if (count % 3 == 0) { status1_green(); }
	else if (count % 3 == 1) { status1_red(); }
	else{ status1_off(); }	
}

/********************************************/
/*         MIDI Rx INTERRUPT                */
/********************************************/
ISR(USART_RX_vect) {
	uint8_t latest_byte = UDR0;
	
	if (mctl.incoming_message(latest_byte))
	{
		// nothing to do...
	}
	else
	{
		// TODO: Remove this!! this is just to detect if the buffer overflows
		status2_red();
		_delay_ms(1000);
	}
}

/******************	   SWITCH DEBOUNCE & CLEAR TRIGGER TIMER   ******************/
/*
	Interrupt is triggered every 1 millisecond.
	If the trigger has been high for TRIG_DUR, then trigger is cleared.
	Mode switch is updated in the time interval defined by SWITCH_DEBOUNCE_DIR. */
/********************************************************************************/
ISR(TIMER1_COMPA_vect) {
	mctl.timer_tick();
}

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
	TCCR0B |= (1 << CS01);					// set prescaler to 8 and start PWM
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
	DDRC = _BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC4) | _BV(DDC5);
	DDRD |= _BV(DDD2) | _BV(DDD3);
}

int main()
{
	init_led_outputs();		/* for debugging */
	init_digital_outputs();	/* advance clock out, trigger out x2,  ...velocities? */
	init_midi_UART();		/* MIDI on the UART Tx/Rx pins */
	init_DAC_SPI();			/* for sending commands to the DAC */
	
	/* configure timers/counters and interrupts */
	init_pwm_output();
	init_timer_interrupt();
	
	DDRC &= ~_BV(DDC3); // set PC3 as input
	DDRB &= ~_BV(DDB1); // set PB1 as input
	
	register_midi_events();
	
	uint16_t idx = 0;
	while (1)
	{
		running_status(idx);
		mctl.check_for_MIDI();
		idx++;
	}
	return 0;
}


/*
	Unfortunately we since we cannot include <functional> we cannot call std::bind or
	anything else to allow us to pass instance methods as callbacks... boo hoo.
	
	Instead of attaching the event handlers inside the MidiController using elegant
	lambdas that capture 'this' and bind it to the callback function, we have the
	following ugliness:
*/
void register_midi_events()
{
	mctl.midi.setHandleControlChange(handleCC);
	mctl.midi.setHandleNoteOn(handleNoteOn);
	mctl.midi.setHandleStart(handleStart);
	mctl.midi.setHandleStop(handleStop);
	mctl.midi.setHandleClock(handleClock);
	mctl.midi.setHandleContinue(handleContinue);
}

void handleCC(byte channel, byte cc_num, byte cc_val)		{ mctl.handleCC(channel, cc_num, cc_val); }
void handleNoteOn(byte channel, byte pitch, byte velocity)	{ mctl.handleNoteOn(channel, pitch, velocity); }
void handleStart()											{ mctl.handleStart(); }
void handleStop()											{ mctl.handleStop(); }
void handleClock()											{ mctl.handleClock(); }
void handleContinue()										{ mctl.handleContinue(); }