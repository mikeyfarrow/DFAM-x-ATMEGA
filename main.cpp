/*
 * FILE: main.cpp
 *
 * This file controls the event loop which is responsible for receiving and
 * transmitting MIDI messages. The event loop also handles analog IO via
 * ATMEGA's GPIO and the SPI communication with the DAC.
 */

#define F_CPU 16000000UL

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

enum UiMode { MidiRx, LearnChannel, LearnKeyboard, CalibrateVoct };


#define DEBOUNCE_DELAY 50
#define LONG_PRESS_DURATION 1000

uint32_t last_debounce_time = 0;
uint32_t button_press_time = 0;
bool button_state = false;
bool last_button_state = false;
bool long_press_fired = false;

/** Instantiate the controller **/
MidiController mctl;

UiMode mode = MidiRx;

uint8_t calibration_count = 0;
const uint8_t default_channels[3] = {1, 2, 10};
uint8_t channel_prefs[3] {}; // CV A, CV B, KCS
uint8_t channel_count = 0;

uint8_t keyboard_prefs[8] {};
uint8_t key_pref_count = 0;

// Event handlers
void register_midi_events();
void unregister_midi_events();
void learn_channel_note_on(uint8_t midi_note);
void learn_channel_progress();

void learn_sw_single_click()
{
	if (mode == LearnChannel || mode == LearnKeyboard)
	{
		/* exit without saving */
		channel_count = 0;
		uint8_t key_pref_count = 0;
		register_midi_events();
		leda_off();
		ledb_off();
		ledc_off();
		mode = MidiRx;
	}
	else if (mode == CalibrateVoct) 
	{
		// todo: advance to next octave/cal. point
		calibration_count++;
	}
	else {
		/* 
			ENTERING "learn channel" mode - MIDI events will be intercepted
			until we receive three Note On messages or until we get another
			single click (exit without saving) 
		*/
		unregister_midi_events();
		mode = LearnChannel;
		channel_count = 0;
		
		leda_green();
		ledb_red();
		ledc_red();
	}
}

void learn_sw_long_press()
{
	mode = LearnKeyboard;
	unregister_midi_events();
	leda_red();
	ledb_red();
	ledc_red();
	mctl.advance_to_beginning();
}

void check_learn_switch()
{
	uint32_t now = mctl.millis();
	bool button_pressed = !bit_is_set(LEARN_SW_PIN, LEARN_SW);

	if (button_pressed != last_button_state)
	{
		last_debounce_time = now;
	}

	if (now - last_debounce_time > DEBOUNCE_DELAY)
	{
		if (button_pressed != button_state)
		{
			button_state = button_pressed;
			if (button_state)
			{ 
				button_press_time = now;
				long_press_fired = false;
			}
			else // Button released
			{  
				if (!long_press_fired)
				{
					learn_sw_single_click();
				}
			}
		}
	}

	if (button_state && !long_press_fired && (now - button_press_time >= LONG_PRESS_DURATION))
	{
		learn_sw_long_press();
		long_press_fired = true;
	}

	last_button_state = button_pressed;
}

int main()
{
	cli(); // disable interrupts globally
	mctl.midi.turnThruOff();
	hardware_init();
	
	if (!load_config(mctl))
		save_config(mctl);
	ledc_off();
	
	register_midi_events();
	sei(); // enable interrupts globally
	
	
	bool learn_held = !bit_is_set(LEARN_SW_PIN, LEARN_SW);
	bool mode_held = !bit_is_set(SYNC_BTN_PIN, SYNC_BTN);
	if (learn_held && mode_held)
	{
		mode = CalibrateVoct;
		calibration_count = 0;
	}
	
	uint16_t idx = 0;
	while (1)
	{
		check_learn_switch();
		if (mode == MidiRx)
		{
			// LEDTODO: if (idx % 2 == 0) { status1_green(); }
			// LEDTODO: else { status1_red(); }
			
			mctl.update();	
		}
		else if (mode == CalibrateVoct)
		{
			leda_green();
			ledb_green();
			ledc_green();
			// TODO: implement calibration procedure...
			// probably best to keep track of state inside CvOutput
		}
		else
		{
			mctl.midi.read();
		}
		idx++;
	}
	return 0;
}


/**************************************************/
/*			Learn CHANNEL Mode					  */
/**************************************************/
void learn_channel_progress()
{
	if (channel_count == 1)
	{
		leda_red();
		ledb_green();
		ledc_red();
	}
	else if (channel_count == 2)
	{
		leda_red();
		ledb_red();
		ledc_green();
	}
	else if (channel_count == 3)
	{
		// we are leaving channel select mode
		mctl.update_midi_channels(channel_prefs);
			
		save_config(mctl); /* save channels (and all other config) to EEPROM */
			
		register_midi_events();
		mode = MidiRx;
		
		leda_off();
		ledb_off();
		ledc_off();
	}
}

void learn_channel_note_on(uint8_t midi_note)
{
	uint8_t ch = midi_note - MIDDLE_C + 1;
	if (ch < 0 || ch >= NUM_CHANNELS)
	{
		ch = default_channels[channel_count];
	}
	channel_prefs[channel_count] = ch;
	
	channel_count++;
	learn_channel_progress();
}


/**************************************************/
/*			Learn KEYBOARD Mode					  */
/**************************************************/
void learn_keyboard_note_on(uint8_t midi_note)
{
	keyboard_prefs[key_pref_count] = midi_note;
	key_pref_count++;
	
	mctl.advance_clock();
	
	if (key_pref_count == DFAM_STEPS)
	{
		// we are leaving channel select mode
		mctl.update_keyboard_prefs(keyboard_prefs);
		
		save_config(mctl); /* save channels (and all other config) to EEPROM */
		
		register_midi_events();
		mode = MidiRx;
		leda_off();
		ledb_off();
		ledc_off();
	}
}


/**************************************************/
/*  INTERRUPTS: TIMERS, MIDI Rx, MIDI Tx ready    */
/**************************************************/

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
	leda_off();
	DISABLE_OCI1A();
}

ISR(TIMER1_COMPB_vect) {
	clear_bit(TRIG_PORT, TRIG_B_OUT);
	ledb_off();
	DISABLE_OCI1B();
}


void handleNoteOn(byte ch, byte pitch, byte vel) 
{
	switch (mode)
	{
		case LearnChannel:
			learn_channel_note_on(pitch);
			break;
			
		case LearnKeyboard:
			learn_keyboard_note_on(pitch);
			break;
			
		default:
			mctl.handleNoteOn(ch, pitch, vel);
			break;
	}
}

void handleCC(byte ch, byte cc_num, byte cc_val) { mctl.handleCC(ch, cc_num, cc_val); }
void handleNoteOff(byte ch, byte pitch, byte vel) { mctl.handleNoteOff(ch, pitch, vel); }
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

void unregister_midi_events()
{
	mctl.midi.setHandleNoteOn(handleNoteOn);
	mctl.midi.setHandleControlChange(nullptr);
	mctl.midi.setHandleStart(nullptr);
	mctl.midi.setHandleStop(nullptr);
	mctl.midi.setHandleClock(nullptr);
	mctl.midi.setHandleContinue(nullptr);
	mctl.midi.setHandlePitchBend(nullptr);
	mctl.midi.setHandleNoteOff(nullptr);
}