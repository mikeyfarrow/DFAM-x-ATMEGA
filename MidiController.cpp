/*
* MidiController.cpp
*
* Created: 7/10/2024 1:00:25 PM
* Author: mikey
*/
#include <avr/io.h>
#include <util/atomic.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include "GPIO.h"
#include "MidiController.h"
#include "lib/MIDI.h"

#define SWITCH_DEBOUNCE_DUR 20  // count of Timer1 interrupts b
#define MAX_ADV_LENGTH 1000 // millis

#define KCS_MODE !switch_state
#define CCS_MODE switch_state

#define NUM_STEPS 8
#define PPQN 24
#define MIDI_ROOT_NOTE 48  // an octave below middle C
#define NUM_DIVISIONS 7
const uint8_t DIVISIONS[NUM_DIVISIONS] = {1, 2, 3, 4, 6, 8, 12};

/************************************************************************/
/*		PUBLIC METHODS		                                            */
/************************************************************************/

/*
	MidiController - default constructor
		initializes the serial transport and MIDI interface
		handles all MIDI events and sequencer logic
*/
MidiController::MidiController():
	transport(),
	clock_period_buffer(),
	cv_out_a(*this, 0),
	cv_out_b(*this, 1),
	midi((SMT&) transport)
{
	last_clock = 0;
	
	millis_last = 0;
	last_sw_read = 0;
	
	follow_midi_clock = false;
	clock_count = 0;
	cur_dfam_step = 0; // the number of the last DFAM step triggered
	switch_state = -1;
	
	time_counter = 0;
}

void MidiController::update_midi_channels(uint8_t* ch)
{
	settings.midi_ch_A = ch[0];
	settings.midi_ch_B = ch[1];
	settings.midi_ch_KCS = ch[2];
}

float MidiController::avg_midi_clock_period()
{
	float sum = 0;
	for (int i = 0; i < BPM_BUFFER_SIZE; i++)
	{
		sum += clock_period_buffer.buffer[i];
	}
	return sum  / BPM_BUFFER_SIZE;
}

void MidiController::update_keyboard_prefs(uint8_t* key_prefs)
{
	for (int i = 0; i < DFAM_STEPS; i++)
	{
		settings.keyboard_step_table[i] = key_prefs[i];
	}
}

void MidiController::time_inc()
{
	time_counter++;
}

uint32_t MidiController::millis()
{
	uint32_t tms;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		tms = time_counter;
	}
	return tms;
}

void MidiController::update()
{
	// check the MIDI Tx buffer and deal with any new messages.
	midi.read();

	// update V/oct outputs based on slide and/or vibrato progress
	cv_out_a.slide_progress();
	cv_out_b.slide_progress();
	
	// read the hardware inputs (the two switches)
	if (millis() - last_sw_read >= SWITCH_DEBOUNCE_DUR)
	{
		check_mode_switch();
		check_sync_switch();
		last_sw_read = millis();
	}
}

void MidiController::tx_ready()
{
	/*
		tx_ready - called to notify the MIDI controller that the USART data register
			is empty and so is ready for a MIDI Tx
	*/
	uint8_t midi_byte;
	if (transport.midi_tx_buffer.get(&midi_byte))
	{
		UDR0 = midi_byte;
	}
}

uint8_t MidiController::incoming_message(uint8_t msg)
{
	return transport.midi_rx_buffer.put(msg);
}

/************************************************************************/
/*		HARDWARE OUTPUT                                                 */
/************************************************************************/

/*
	advance_clock - sends a single pulse on the ADV/CLOCK output
*/
void MidiController::advance_clock()
{
	set_bit(ADV_PORT, ADV_OUT);
	
	for (int i = 0; i < settings.adv_clock_ticks; i++)
	{
		/* do nothing, wait a while */
		_NOP();
	}
	
	clear_bit(ADV_PORT, ADV_OUT);
}

/*
	advance_clock - sends a number of pulses on the ADV/CLOCK output
*/
void MidiController::advance_clock(uint8_t steps)
{
	for (int i = 0; i < steps; i++)
	{
		advance_clock();
	}
}

/*
check_mode_switch -
There are two modes: CCS (clock-controlled sequencer) or KCS (keyboard-controlled
sequencer). SWITCH_STATE is true for CCS, false for KCS.

When the mode switch is flipped we are either switching
- from CCS into KCS (sequence could be in Play or Stop)
- from KCS into CCS
Either way, restart the clock counter and set the DFAM sequencer back to step 1.
*/
void MidiController::check_mode_switch()
{
	uint8_t cur_switch = bit_is_set(MODE_SWITCH_PIN, MODE_SWITCH);
	if (cur_switch == switch_state)
	{
		return;
	}

	switch_state = cur_switch;
	clock_count = 0;
	
	int steps_left = steps_between(cur_dfam_step, 1) + 1;
	advance_clock(steps_left);

	if (switch_state) // switched into "clock-controlled sequencer" mode
	{
		//ledc_green();
		cur_dfam_step = 0;
	}
	else			  // switched into "keyboard-controlled sequencer" mode
	{
		//ledc_red();
		follow_midi_clock = false;
		cur_dfam_step = 1;
	}
}

void MidiController::advance_to_beginning()
{
	int steps_left = steps_between(cur_dfam_step, 1) + 1;
	advance_clock(steps_left);
	// LEDTODO: clear_bit(LED_BANK_PORT, LED1);
	follow_midi_clock = false;
	cur_dfam_step = 1;
}

/*
	check_sync_switch - in case the DFAM sequencer has gotten out of sync
		with CUR_DFAM_STEP, restart it.
*/
void MidiController::check_sync_switch()
{
	if (!bit_is_set(SYNC_BTN_PIN, SYNC_BTN))
	{
		cur_dfam_step = 1;
	}
}

/************************************************************************/
/*		EVENT HANDLERS                                                  */
/************************************************************************/

#define CC_AdvClockWidth  MIDI_NAMESPACE::GeneralPurposeController7 // cc #82
#define CC_ClockDiv		  MIDI_NAMESPACE::GeneralPurposeController8 // cc #83

void MidiController::handleCC(byte channel, byte cc_num, byte cc_val)
{
	if (channel == settings.midi_ch_A)
		cv_out_a.control_change(cc_num, cc_val);
	
	if (channel == settings.midi_ch_B)
		cv_out_b.control_change(cc_num, cc_val);

	/* considering these CCs as "Global" for now ... */
	switch (cc_num)
	{
		case CC_AdvClockWidth:
			settings.adv_clock_ticks = (cc_val * MAX_ADV_LENGTH / 127.0);
			break;
			
		case CC_ClockDiv:
			settings.clock_div = DIVISIONS[(uint8_t) (cc_val * NUM_DIVISIONS / 127.0)];
			break;
		
		case MIDI_NAMESPACE::OmniModeOff:
			break;
		
		case MIDI_NAMESPACE::OmniModeOn:
			break;
		
		case MIDI_NAMESPACE::MonoModeOn:
			settings.midi_mode = Mono;
			break;
		
		case MIDI_NAMESPACE::PolyModeOn:
			settings.midi_mode = Poly;
			break;
		
		case MIDI_NAMESPACE::AllNotesOff:
			cv_out_a.all_notes_off();
			cv_out_b.all_notes_off();
			break;

		case MIDI_NAMESPACE::ResetAllControllers:
			break;
		
		default:
			break;
	}
}

void MidiController::handleNoteOn(uint8_t channel, uint8_t midi_note, uint8_t velocity)
{
	if (settings.midi_mode == Poly && channel == settings.midi_ch_A)
	{
		if (cv_out_a.latest() == -1) 
		{	// No note is held on Cv_A: we will assign it its first note.
			// It will not be assigned a new note until this one is released.
			cv_out_a.note_on(midi_note, velocity, true, true);
		}
		else
		{   // Assign all other notes to Cv_B - this means B is the only
			// one to have voices "stolen". It also means B is the only one
			// to that will ever have a chance to to do any re-trigger behavior.
			cv_out_b.note_on(midi_note, velocity, true, true);
		}
		
		// trigger A on every note on, regardless of voice allocation
		cv_out_a.trigger();
	}
	
	if (settings.midi_mode == Mono)
	{
		if (channel == settings.midi_ch_A)
			cv_out_a.note_on(midi_note, velocity, true, true);
		
		if (channel == settings.midi_ch_B) // only send vel. B in CCS mode
			cv_out_b.note_on(midi_note, velocity, CCS_MODE, true);
	}

	if (channel == settings.midi_ch_KCS && KCS_MODE)
	{
		uint8_t dfam_step = midi_note_to_step(midi_note);
		if (dfam_step) {
			VEL_B_DUTY = velocity << 1;
			int steps_left = steps_between(cur_dfam_step, dfam_step) + 1;
			advance_clock(steps_left);
			cur_dfam_step = dfam_step;
		}
	}
}

void MidiController::handleNoteOff(uint8_t channel, uint8_t midi_note, uint8_t velocity)
{
	if (settings.midi_mode == Mono)
	{
		if (channel == settings.midi_ch_A)
			cv_out_a.note_off(midi_note, velocity);
	
		if (channel == settings.midi_ch_B)
			cv_out_b.note_off(midi_note, velocity);
	}
	else /* midi_mode == Poly */
	{
		if (cv_out_a.notes_held[midi_note])
		{
			cv_out_a.note_off(midi_note, velocity);
			if (cv_out_a.latest() == -1)
				clear_bit(TRIG_PORT, TRIG_A_OUT);
		}
		else
		{
			cv_out_b.note_off(midi_note, velocity);
			if (cv_out_b.latest() == -1)
				clear_bit(TRIG_PORT, TRIG_B_OUT);
		}
	}
}

/*
	handleStart - handles beginning a sequence
*/
void MidiController::handleStart()
{
	if (switch_state)
	{
		uint8_t steps_left = steps_between(cur_dfam_step, 1);
		advance_clock(steps_left);
		follow_midi_clock = true;
		clock_count = 0;
		cur_dfam_step = 0;
	}
}

void MidiController::handleStop()
{
	follow_midi_clock = false;
}

/*
	handleClock - 
*/
void MidiController::handleClock()
{
	//uint32_t now = millis() / 2;
	//uint32_t period_ms = now - last_clock;
	//last_clock = now;
	//clock_period_buffer.put(period_ms);
	
	if (follow_midi_clock && switch_state)
	{
		// only count clock pulses while sequence is playing and CCS mode is selected
		clock_count = clock_count % (PPQN / settings.clock_div) + 1;

		if (clock_count == 1) // we have a new step
		{
			cur_dfam_step = cur_dfam_step % NUM_STEPS + 1;
			advance_clock();
			ledc_green();
		}
		else
		{
			ledc_off();
		}
	}
}

/*
	handleContinue - 
*/
void MidiController::handleContinue()
{   
	follow_midi_clock = true;
}

/*
	handlePitchBend - amt is in the range -8192 to 8191
*/
void MidiController::handlePitchBend(byte midi_ch, int16_t amt)
{
	if (midi_ch == settings.midi_ch_A)
		cv_out_a.pitch_bend(amt);
	
	if (midi_ch == settings.midi_ch_B) // only send vel. B in CCS mode
		cv_out_b.pitch_bend(amt);
}

/************************************************************************/
/*		HELPER METHODS                                                  */
/************************************************************************/

/*
 * steps_between - returns the number of steps the DFAM sequencer would need to 
 *			advance to get from start to end
 */
uint8_t MidiController::steps_between(int start, int end)
{
	if (start == 0 || start == end)
		return NUM_STEPS - 1;

	int steps_left = end - start - 1;
	if (end < start)
		steps_left += NUM_STEPS;
	
	return steps_left;
}

/*
	midi_note_to_step - used in "8-voice monosynth" mode to determine how
		many steps to advance to get to the chosen note
*/
uint8_t MidiController::midi_note_to_step(uint8_t note) {
	for (int i = 0; i < NUM_STEPS; i++) {
		if (note == settings.keyboard_step_table[i])
			return i + 1;
	}
	return false;
}

