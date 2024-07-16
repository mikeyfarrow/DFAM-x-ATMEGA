/*
 * FILE: main.cpp
 *
 * This file controls the event loop which is responsible for receiving and
 * transmitting MIDI messages. The event loop also handles analog IO via
 * ATMEGA's GPIO and the SPI communication with the DAC.
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "GPIO.h"
#include "MidiController.h"

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

int main()
{
	cli(); // disable interrupts globally
	
	mctl.midi.turnThruOff();
	
	init_led_outputs();		/* for debugging */
	init_digital_outputs();	/* advance clock out, trigger out x2,  ...velocities? */
	init_midi_UART();		/* MIDI on the UART Tx/Rx pins */
	init_DAC_SPI();			/* for sending commands to the DAC */
	
	/* configure timers/counters and interrupts */
	init_pwm_output();
	init_timer_interrupt_1s();
	init_timer_interrupt_1ms();
	
	DDRC &= ~_BV(DDC3); // set PC3 as input
	DDRB &= ~_BV(DDB1); // set PB1 as input
	DDRD |= _BV(DDD1); // set PD1 (Tx) as output
	
	register_midi_events();
	
	sei(); // enable interrupts globally
	
	uint16_t idx = 0;
	while (1)
	{
		if (idx % 3 == 0) { status1_green(); }
		else if (idx % 3 == 1) { status1_red(); }
		else{ status1_off(); }
		
		mctl.check_for_MIDI();
		idx++;
	}
	return 0;
}


/**************************************************/
/*  INTERRUPTS: TIMERS, MIDI Rx, MIDI Tx ready    */
/**************************************************/

//  MIDI Tx is ready (i.e. "Data Register Empty") - tell the MidiController
ISR(USART_UDRE_vect) {
	mctl.tx_ready();
}

// MIDI Rx message - there is a new byte in the data register
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

// Timer Interrupt: every 1 ms
ISR(TIMER2_COMPA_vect) {
	mctl.timer_tick();
}

// Timer Interrupt: every 1 sec
ISR(TIMER1_COMPA_vect) {
	//mctl.midi.sendNoteOn(55, 127, 1);
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