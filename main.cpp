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

void check_mode_switch()
{
	uint8_t cur_switch = bit_is_set(PINC, PINC3);
	if (cur_switch == SWITCH_STATE)
		return;

	SWITCH_STATE = cur_switch;
	CLOCK_COUNT = 0;

	int steps_left = steps_between(CUR_DFAM_STEP, 1) + 1;
	advance_clock(steps_left);

	if (SWITCH_STATE)
	{
		FOLLOW_MIDI_CLOCK = true;
		CUR_DFAM_STEP = 0;
		bank(0);
	}
	else
	{
		clear_bank();
		bank_off(0);
		//handleStop();
	}
}

/******   SWITCH DEBOUNCE TIMER   ******/
ISR(TIMER1_COMPA_vect) { 
	check_mode_switch();
}

void init_timer_interrupts()
{
	// disable interrupts globally
	cli(); 
	
	// see: http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
	
	/**************************************/
	/*********   TIMER 0 config   *********/
	TCCR0A = 0;											// clear timer registers
	TCCR0B = 0;
	TCNT0 = 0;											// reset the counter
	OCR0A = 249;										// 1000 Hz i.e. every 1 ms
	TCCR0A |= (1 << WGM01);								// turn on CTC mode
	TCCR0B |= (1 << CS01) | (1 << CS00);				// prescaler = 64
	TIMSK0 |= (1 << OCIE0A);							// Output Compare Match A Interrupt Enable
	
	
	/**************************************/
	/*********   TIMER 1 config   *********/
	TCCR1A = 0;											// clear timer registers
	TCCR1B = 0;
	TCNT1  = 0;											// reset the counter
	OCR1A = 39999;										// frequency 50 Hz i.e. every 20 ms
	TCCR1B |= (1 << WGM12);								// turn on CTC mode
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);  // prescaler = 8
	TIMSK1 |= (1 << OCIE1A);							// enable timer compare interrupt
	
	// enable interrupts globally
	sei();
}

int main()
{
	init_led_outputs();				/* for debugging */
	init_digital_outputs();			/* advance clock out, trigger out x2,  ...velocities? */
	serialMIDI.init_midi_UART();	/* MIDI on the UART Tx/Rx pins */
	init_DAC_SPI();					/* for sending commands to the DAC */
	
	init_timer_interrupts();
	
	DDRC &= ~_BV(DDC3); // set PC3 as input
	
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