/*
 * FILE: main.cpp
 *
 * This file controls the event loop which is responsible for receiving and
 * transmitting MIDI messages. The event loop also handles analog IO via
 * ATMEGA's GPIO and the SPI communication with the DAC.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "GPIO.h"
#include "MidiController.h"

#define MOMENTARY_SW_DEBOUNCE_MS 500
#define MIDDLE_C 60
#define NUM_CHANNELS 16

/** Instantiate the controller **/
MidiController mctl;

/*

CHANNEL SELECT MODE - when enabled, the program waits for three note on events from ANY channel
	the note values correspond to midi channels:
		- C4 (i.e. middle C, note number 60) is MIDI channel 1
		- +1 semitone is +1 channel
		- D#5 is ch 16 (midi note number 75
		- if you play a note outside of this range, the default basic channel is used
*/

uint32_t last_momentary_press = 0;
bool channel_select_mode = false;
uint8_t default_channels[3] = {1, 2, 10};
uint8_t channels[3] = {0, 0, 0}; // 0->CV A, 1->CV B, 2->KCS
uint8_t channel_count = 0;


// Event handlers
void register_midi_events();
void handleCC(byte channel, byte cc_num, byte cc_val);
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleStart();
void handleStop();
void handleClock();
void handleContinue();
void handleNoteOff(byte ch, byte pitch, byte vel);
void handlePitchBend(byte ch, int amt);

int main()
{
	cli(); // disable interrupts globally
	
	mctl.midi.turnThruOff();
	
	/* hardware/IO initialization */
	init_led_outputs();
	init_digital_outputs();
	init_midi_UART();
	init_DAC_SPI();
	
	/* configure timers/counters and interrupts */
	init_pwm_output();
	init_milli_counter_timer();
	init_timer1();
	
	DDRB &= ~_BV(DDB4); // PB4 is input
	DDRC &= ~_BV(DDC3); // set PC3 as input
	DDRB &= ~_BV(DDB1); // set PB1 as input
	DDRD |= _BV(DDD1); // set PD1 (Tx) as output
	
	/* pin change interrupt for "channel select" mode switch */
	PCMSK0 |= (1<<PCINT4);
	PCICR |= (1<<PCIE0);
		
	register_midi_events();
	
	sei(); // enable interrupts globally
	
	uint16_t idx = 0;
	status2_off();
	while (1)
	{
		if (channel_select_mode)
		{
			mctl.midi.read();
		}
		else
		{
			if (idx % 2 == 0) { status1_green(); }
			else { status1_red(); }
			mctl.update();	
		}
		
		
		idx++;
	}
	return 0;
}


/**************************************************/
/*  INTERRUPTS: TIMERS, MIDI Rx, MIDI Tx ready    */
/**************************************************/

////  MIDI Tx is ready (i.e. "Data Register Empty") - tell the MidiController
//ISR(USART_UDRE_vect) {
	//mctl.tx_ready();
//}

// MIDI Rx message - there is a new byte in the data register
ISR(USART_RX_vect) {
	uint8_t latest_byte = UDR0;
	mctl.incoming_message(latest_byte);
}

// measured frequency of LED toggle = 94.7 kHz (on the DMM)
ISR(TIMER2_COMPA_vect) {
	mctl.time_inc();
}

ISR(TIMER1_COMPA_vect) {
	clear_bit(TRIG_PORT, TRIG_A_OUT);
	DISABLE_OCI1A();
}

ISR(TIMER1_COMPB_vect) {
	clear_bit(TRIG_PORT, TRIG_B_OUT);
	DISABLE_OCI1B();
}

ISR(PCINT0_vect){
	if (mctl.millis() - last_momentary_press > MOMENTARY_SW_DEBOUNCE_MS)
	{
		last_momentary_press = mctl.millis();
		if (bit_is_set(PINB, PINB3) && !channel_select_mode)
		{
			channel_select_mode = true;
			channel_count = 0;
			status1_red();
			status2_red();
		}
	}
}

void register_midi_events()
{
	mctl.midi.setHandleControlChange(handleCC);
	mctl.midi.setHandleNoteOn(handleNoteOn);
	mctl.midi.setHandleStart(handleStart);
	mctl.midi.setHandleStop(handleStop);
	mctl.midi.setHandleClock(handleClock);
	mctl.midi.setHandleContinue(handleContinue);
	mctl.midi.setHandlePitchBend(handlePitchBend);
	mctl.midi.setHandleNoteOff(handleNoteOff);
}

/*
	Unfortunately we since we cannot include <functional> we cannot call std::bind or
	anything else to allow us to pass instance methods as callbacks... boo hoo.
	
	Instead of attaching the event handlers inside the MidiController using elegant
	lambdas that capture 'this' and bind it to the callback function, we have the
	following ugliness:
*/
void handleCC(byte ch, byte cc_num, byte cc_val)   { mctl.handleCC(ch, cc_num, cc_val); }

void handleNoteOff(byte ch, byte pitch, byte vel)  { /*mctl.handleNoteOff(ch, pitch, vel); */ }

void channel_select_progress_led()
{
	if (channel_count == 1)
	{
		status2_green();
	}
	else if (channel_count == 2)
	{
		status1_green();
	}
}

void store_channel_selections()
{
	// write to EEPROM
	// ... TODO...
	//     will also need to read the EEPROM on boot?
	
	
	// update the MIDI controller
	mctl.update_midi_channels(channels);
}

void channel_select_note(uint8_t midi_note)
{
	uint8_t ch = midi_note - MIDDLE_C + 1;
	if (ch < 0 || ch >= NUM_CHANNELS)
	{
		ch = default_channels[channel_count];
	}
	channels[channel_count] = ch;
	
	channel_count++;
	channel_select_progress_led();

	if (channel_count == 3)
	{
		// we are leaving channel select mode
		status1_off();
		status2_off();
		store_channel_selections();
		channel_select_mode = false;
	}
}

void handleNoteOn(byte ch, byte pitch, byte vel) 
{
	if (channel_select_mode)
	{
		channel_select_note(pitch);
	}
	else
	{
		mctl.handleNoteOn(ch, pitch, vel);
	}
}

void handleStart()								  { mctl.handleStart(); }

void handleStop()								  { mctl.handleStop(); }

void handleClock()								  { mctl.handleClock(); }

void handleContinue()							  { mctl.handleContinue(); }

void handlePitchBend(byte ch, int amt)			  { mctl.handlePitchBend(ch, amt); }