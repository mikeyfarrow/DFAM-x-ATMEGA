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

#include "main.h"
#include "GPIO.h"
#include "MidiController.h"
#include "EEPromManager.h"

#define MOMENTARY_SW_DEBOUNCE_MS 500
#define MIDDLE_C 60
#define NUM_CHANNELS 16
#define DEBOUNCE_DELAY 50
#define LONG_PRESS_DURATION 1000

enum UiMode { MidiRx, LearnChannel, LearnKeyboard, CalibrateVoct };

MidiController mctl;
UiMode mode = MidiRx;

/* "Learn" button state */
uint32_t last_debounce_time = 0;
uint32_t button_press_time = 0;
bool button_state = false;
bool last_button_state = false;
bool long_press_fired = false;

/* "Learn" CV Midi Channel state */
const uint8_t default_channels[3] = {1, 2, 10};
uint8_t channel_prefs[3] {}; // CV A, CV B, KCS
uint8_t channel_count = 0;

/* "Learn" Keyboard Mode key mapping state */
uint8_t keyboard_prefs[8] {};
uint8_t key_pref_count = 0;

/* V/oct Calibration state */
uint8_t		k_voct_cal		= 0;	/* Number of octaves calibrated, aka current index in the calibration array */
float		cur_cal_offset	= 0.0;  /* Value used to adjust the current calibration point */
uint32_t	last_blink		= 0;    /* Last time the CvOutput LED blinked (to indicate which octave is being calibrated */
uint8_t		k_blink			= 0;	/* Number of times LED has blinked */
uint8_t		k_blink_wait	= 0;	/* Number of cycles to wait between sequences of blinks */

/* the CV output that is currently being calibrated */
CvOutput*	cal_output		= &mctl.cv_out_a; 

int main()
{
	cli(); // disable interrupts globally
	
	mctl.midi.turnThruOff();
	hardware_init();
	
	if (!load_config(mctl))
	save_config(mctl);
	ledc_off();
	
	register_midi_events();
	check_calibration_mode();
	
	sei(); // enable interrupts globally
	
	mctl.cv_out_a.settings.trigger_duration_ms = 10;
	mctl.cv_out_b.settings.trigger_duration_ms = 10;
	mctl.cv_out_a.settings.trig_mode = Gate;
	mctl.cv_out_a.settings.retrig_mode = Highest;
	mctl.cv_out_a.settings.vib_mode = Free;
	
	mctl.settings.midi_ch_A = 1;
	mctl.settings.midi_ch_B = 1;
	
	
	uint16_t idx = 0;
	while (1)
	{
		check_learn_switch();
		if (mode == MidiRx)
		{
			mctl.update();
		}
		else
		{
			mctl.midi.read();
			if (mode == CalibrateVoct)
			{
				update_blink_counter();
			}
		}
		idx++;
	}
	return 0;
}

void resume_midi_rx()
{
	register_midi_events();
	leda_off();
	ledb_off();
	ledc_off();
	mode = MidiRx;
}

void learn_sw_single_click()
{
	if (mode == LearnChannel || mode == LearnKeyboard)
	{
		channel_count = 0;
		uint8_t key_pref_count = 0;
		resume_midi_rx();
	}
	else if (mode == CalibrateVoct) 
	{
		iterate_voct_calibration();
	}
	else
	{
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
	if (mode == CalibrateVoct)
	{
		return; /* ignore long presses when in calibration mode */
	}
	
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

/****************************************/
/*		Calibrate V/oct Mode            */
/****************************************/
void check_calibration_mode()
{
	bool learn_held = !bit_is_set(LEARN_SW_PIN, LEARN_SW);
	bool mode_held = !bit_is_set(SYNC_BTN_PIN, SYNC_BTN);
	if (learn_held && mode_held)
	{
		/* We are in calibration mode. */
		leda_green();
		ledb_green();
		ledc_green();
		
		/* Begin with CvOutput A */
		mode = CalibrateVoct;
		k_voct_cal = 0;
		cur_cal_offset = 0.0;
		
		output_calibration_voltage();
	}
}

void update_blink_counter()
{
	uint32_t now = mctl.millis();
	if (now - last_blink > 800)
	{
		k_blink++;
		last_blink = now;
		
		if (k_blink_wait > 0)
		{
			k_blink_wait--;
			k_blink = 0;
		}
		else
		{
			cal_output->trigger();
		}
		
		if (k_blink >= k_voct_cal)
		{
			k_blink_wait = 2;
		}
	}
}

void calibration_cc(byte cc_num, byte cc_val)
{
	float val = (cc_val / 127.0) - 0.5;
	if (cc_num == 102)
	{
		cur_cal_offset = 15 * val;
	}
	else if (cc_num == 103)
	{
		cur_cal_offset = 3 * val;
	}
	else if (cc_num == 104)
	{
		cur_cal_offset = 25 * val;
	}
	output_calibration_voltage();
}

void output_calibration_voltage()
{
	uint8_t midi_note = k_voct_cal * 12; // 0, 12, 24, ..., 120
	float cur_cal = cal_output->get_calibration_value(k_voct_cal);
	uint8_t note = 127 - midi_note;
	uint16_t dac_data = note * (cur_cal + cur_cal_offset);
	
	cal_output->output_dac(dac_data);
}

void iterate_voct_calibration()
{
	// update the output's settings for this octave
	cal_output->adjust_calibration(k_voct_cal, cur_cal_offset);
		
	// advance to the next octave
	k_voct_cal++;
	cur_cal_offset = 0;
		
	// check if we just completed the output's calibration
	if (k_voct_cal == 11)
	{
		k_voct_cal = 0;
		save_config(mctl);
			
		if (cal_output->dac_ch == 0)
		{
			cal_output = &mctl.cv_out_b;
		}
		else
		{
			resume_midi_rx();
			return;
		}
	}

	output_calibration_voltage();
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
			
		case CalibrateVoct:
			break;
			
		default:
			mctl.handleNoteOn(ch, pitch, vel);
			break;
	}
}

void handleCC(byte ch, byte cc_num, byte cc_val)
{
	switch (mode)
	{
		case CalibrateVoct:
			calibration_cc(cc_num, cc_val);
			break;
			
		case MidiRx:
			mctl.handleCC(ch, cc_num, cc_val);
			break;
		
		default: break;
	}
}

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
	mctl.midi.setHandleControlChange(handleCC);
	mctl.midi.setHandleStart(nullptr);
	mctl.midi.setHandleStop(nullptr);
	mctl.midi.setHandleClock(nullptr);
	mctl.midi.setHandleContinue(nullptr);
	mctl.midi.setHandlePitchBend(nullptr);
	mctl.midi.setHandleNoteOff(nullptr);
}