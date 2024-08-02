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

//#define ENABLE_MIDI_OUTPUT 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "GPIO.h"
#include "MidiController.h"
#include "EEPromManager.h"

#define MOMENTARY_SW_DEBOUNCE_MS 500
#define MIDDLE_C 60
#define NUM_CHANNELS 16

enum UiMode { MidiRx, LearnChannel, LearnKeyboard };

/** Instantiate the controller **/
MidiController mctl;

uint32_t last_momentary_press = 0;

UiMode mode = MidiRx;
uint8_t default_channels[3] = {1, 2, 10};
uint8_t channels[3] = {0, 0, 0}; // 0->CV A, 1->CV B, 2->KCS
uint8_t channel_count = 0;

// Event handlers
void register_midi_events();

int main()
{
	cli(); // disable interrupts globally
	
	mctl.midi.turnThruOff();
	
	/* hardware/IO initialization */
	init_led_outputs();
	init_digital_outputs();
	init_digital_inputs();
	init_midi_UART();
	init_DAC_SPI();
	
#ifdef ENABLE_MIDI_OUTPUT
	UCSR0B |= DATA_REGISTER_EMPTY_INTERRUPT;
	DDRD |= _BV(DDD1); // set PD1 (Tx) as output
#endif
	
	/* configure timers/counters and interrupts */
	init_pwm_output();
	init_milli_counter_timer();
	init_timer1();

	
	/* pin change interrupt for "channel select" mode switch */
	PCMSK0 |= (1<<PCINT4);
	PCICR |= (1<<PCIE0);
		
	register_midi_events();
	
	sei(); // enable interrupts globally
	
	status2_off();
	
	if (!load_config(mctl))
		save_config(mctl);
	
	status2_off();
	
	uint16_t idx = 0;
	while (1)
	{
		if (mode == LearnChannel)
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
/*			Learn CHANNEL Mode					  */
/**************************************************/

void learn_channel_begin()
{
	
}

void learn_channel_progress()
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

void learn_channel_note_on(uint8_t midi_note)
{
	uint8_t ch = midi_note - MIDDLE_C + 1;
	if (ch < 0 || ch >= NUM_CHANNELS)
	{
		ch = default_channels[channel_count];
	}
	channels[channel_count] = ch;
	
	channel_count++;
	learn_channel_progress();

	if (channel_count == 3)
	{
		// we are leaving channel select mode
		status1_off();
		status2_off();
		mctl.update_midi_channels(channels);
		
		save_config(mctl); /* save channels (and all other config) to EEPROM */
		mode = MidiRx;
	}
}


/**************************************************/
/*			Learn KEYBOARD Mode					  */
/**************************************************/

void learn_keyboard_begin()
{
	
}

void learn_keyboard_note_on()
{
	
}


/**************************************************/
/*  INTERRUPTS: TIMERS, MIDI Rx, MIDI Tx ready    */
/**************************************************/

#ifdef ENABLE_MIDI_OUTPUT
//  MIDI Tx is ready (i.e. "Data Register Empty") - tell the MidiController
ISR(USART_UDRE_vect) {
	mctl.tx_ready();
}
#endif

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
		if (bit_is_set(LEARN_SW_PIN, LEARN_SW) && mode != LearnChannel)
		{
			mode = LearnChannel;
			channel_count = 0;
			status1_red();
			status2_red();
		}
	}
}

void handleCC(byte ch, byte cc_num, byte cc_val)   
{
	mctl.handleCC(ch, cc_num, cc_val); 
}

void handleNoteOff(byte ch, byte pitch, byte vel) 
{
	mctl.handleNoteOff(ch, pitch, vel);
}

void handleNoteOn(byte ch, byte pitch, byte vel) 
{
	toggle_bit(LED_BANK_PORT, LED2);
	if (mode == LearnChannel)
	{
		learn_channel_note_on(pitch);
	}
	else
	{
		mctl.handleNoteOn(ch, pitch, vel);
	}
}

void handleStart()						{ mctl.handleStart(); }
void handleStop()						{ mctl.handleStop(); }
void handleClock()						{ mctl.handleClock(); }
void handleContinue()					{ mctl.handleContinue(); }
void handlePitchBend(byte ch, int amt)	{ mctl.handlePitchBend(ch, amt); }
	
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