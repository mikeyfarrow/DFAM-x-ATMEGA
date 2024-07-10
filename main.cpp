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

#define SWITCH_DEBOUNCE_DUR 20  // count of Timer1 interrupts b

/* for debugging */
EEPROM_Writer ew = EEPROM_Writer();

/* create MIDI instances */
SerialTransport serialMIDI;
MidiInterface	MIDI((SerialTransport&) serialMIDI);

volatile uint8_t debounce_ticks = 0;

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

/******************	   SWITCH DEBOUNCE & CLEAR TRIGGER TIMER   ******************/
/*
	Interrupt is triggered every 1 millisecond.
	If the trigger has been high for TRIG_DUR, then trigger is cleared.
	Mode switch is updated in the time interval defined by SWITCH_DEBOUNCE_DIR. */
/********************************************************************************/
ISR(TIMER1_COMPA_vect) {
	trig_A_ticks++;
	trig_B_ticks++;
	debounce_ticks++;
	
	if (trig_A_ticks >= trigger_duration_ms)
	{
		clear_bit(TRIG_PORT, TRIG_A_OUT);
	}
	
	if (trig_B_ticks >= trigger_duration_ms)
	{
		clear_bit(TRIG_PORT, TRIG_B_OUT);
	}
	
	if (debounce_ticks >= SWITCH_DEBOUNCE_DUR)
	{
		debounce_ticks = 0;
		check_mode_switch();
		check_sync_switch();
	}
}

/**********************************************************/
/*     TIMER 0 - fast PWM with outputs on PD6 and PD3     */
/*				 frequency = 8 kHz			     		  */
/* see https://avr-guide.github.io/pwm-on-the-atmega328/  */
/**********************************************************/
void init_pwm_output()
{
	DDRD |= (1 << DDD6) | (1<<DDD5);		// PD6 & PD5 is now an output
	TCCR0A |= (1 << COM0A1) | (1<<COM0B1);	// set none-inverted mode for both output compares
	TCCR0A |= (1 << WGM01) | (1 << WGM00);	// set fast PWM Mode
	OCR0A = 0x00;							// pwm out #1 duty cycle 0
	OCR0B = 0x00;							// pwm out #2 duty cycle 0
	TCCR0B |= (1 << CS01);					// set prescaler to 8 and start PWM
	
}

/*******************************************************************/
/*     TIMER 1 - Interrupt every 1 millisecond                     */
/*														           */
/* http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html */
/*******************************************************************/
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
	
	sei(); // enable interrupts globally
}

int main()
{
	init_led_outputs();				/* for debugging */
	init_digital_outputs();			/* advance clock out, trigger out x2,  ...velocities? */
	serialMIDI.init_midi_UART();	/* MIDI on the UART Tx/Rx pins */
	init_DAC_SPI();					/* for sending commands to the DAC */
	
	init_pwm_output();
	init_timer_interrupt();
	
	DDRC &= ~_BV(DDC3); // set PC3 as input
	DDRB &= ~_BV(DDB1); // set PB1 as input
	
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