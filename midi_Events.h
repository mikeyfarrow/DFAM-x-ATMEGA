/*
 * FILE: midi_Events.h
 *
 * This file defines all functions that act as event handlers/callbacks
 * for incoming MIDI messages.
 *
 */

#ifndef MIDI_EVENTS_H_
#define MIDI_EVENTS_H_

#include "lib/MIDI.h"
#include "lib/midi_Defs.h"

#include "./debug_leds.h"
#include "./MCP_4822.h"
#include "./digital_outputs.h"
#include "./GPIO.h"

#define Channel_T MIDI_NAMESPACE::Channel

#define MIDI_CH_VOCT_A 1	// channel for v/oct on the primary cv out
#define MIDI_CH_VOCT_B 2	// channel for  v/oct on the secondary cv out (can be the same channel)
#define MIDI_CH_KBRD_MODE 3 // MIDI channel for playing DFAM in "8-voice mono-synth" aka KCS mode

#define KCS_MODE !SWITCH_STATE
#define CCS_MODE SWITCH_STATE

/**************************/
/**** Global CONSTANTS ****/
#define NUM_STEPS 8
#define PPQN 24
#define MIDI_ROOT_NOTE 48  // an octave below middle C

#define NUM_DIVISIONS 7
const uint8_t DIVISIONS[NUM_DIVISIONS] = {1, 2, 3, 4, 6, 8, 12};

/**************************/
/**** Global VARIABLES ****/

volatile uint8_t trigger_duration_ms = 2;

uint8_t FOLLOW_MIDI_CLOCK = false;
uint8_t CLOCK_COUNT = 0;
uint8_t CUR_DFAM_STEP = 0; // the number of the last DFAM step triggered
uint8_t CLOCK_DIV = 4;
uint8_t PULSES_PER_STEP = PPQN / CLOCK_DIV;
uint8_t MIDI_CHAN_DFAM = 1; // MIDI channel for playing DFAM in "8-voice mono-synth" mode

volatile uint8_t SWITCH_STATE = -1; // overflow initially so that state is always updated on start up

// by default use the C major scale starting an octave below middle C
uint8_t KEYBOARD_STEP_TABLE[8] = {48, 50, 52, 53, 55, 57, 59, 60};

/************************************************************************/
/*			  HELPER FUNCTIONS                                          */
/************************************************************************/

void handleStop();

/*
 * steps_between - returns the number of steps the DFAM sequencer would need to 
 *			advance to get from start to end
 */
uint8_t steps_between(int start, int end)
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
uint8_t midi_note_to_step(uint8_t note) {
	for (int i = 0; i < NUM_STEPS; i++) {
		if (note == KEYBOARD_STEP_TABLE[i])
			return i + 1;
	}
	return false;
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
void check_mode_switch()
{
	uint8_t cur_switch = bit_is_set(MODE_SWITCH_PIN, MODE_SWITCH);
	if (cur_switch == SWITCH_STATE)
		return;

	SWITCH_STATE = cur_switch;
	CLOCK_COUNT = 0;
	
	int steps_left = steps_between(CUR_DFAM_STEP, 1) + 1;
	advance_clock(steps_left);

	if (SWITCH_STATE) // switched into "clock-controlled sequencer" mode
	{
		bank(0);
		CUR_DFAM_STEP = 0;
	}
	else			  // switched into "keyboard-controlled sequencer" mode
	{
		bank_off(0);
		FOLLOW_MIDI_CLOCK = false;
		CUR_DFAM_STEP = 1;
	}
}

/*
	check_sync_switch - in case the DFAM sequencer has gotten out of sync
		with CUR_DFAM_STEP, restart it back at 0.
*/
void check_sync_switch()
{
	if (!bit_is_set(SYNC_BTN_PIN, SYNC_BTN))
	{
		CUR_DFAM_STEP = 1;
	}
}

/************************************************************************/
/*			  EVENT HANDLERS                                            */
/************************************************************************/

#define CC_TRIG_LENGTH MIDI_NAMESPACE::GeneralPurposeController1
#define CC_ADV_WIDTH MIDI_NAMESPACE::GeneralPurposeController2
#define CC_CLOCK_DIV MIDI_NAMESPACE::GeneralPurposeController3

void handleCC(Channel_T channel, byte cc_num, byte cc_val )
{
	switch (cc_num)
	{
		case MIDI_NAMESPACE::ModulationWheel:
			break;
		
		case CC_TRIG_LENGTH:
			trigger_duration_ms = (cc_val * MAX_TRIG_LENGTH / 127.0);
			break;
			
		case CC_ADV_WIDTH:
			adv_clock_ticks = (cc_val * MAX_ADV_LENGTH / 127.0);
			break;
			
		case CC_CLOCK_DIV:
		{
			uint8_t div_idx = (int) cc_val * NUM_DIVISIONS / 127.0;
			CLOCK_DIV = DIVISIONS[div_idx];
			break;
		}
			
		default:
			break;
	}
}

void handleNoteOn(Channel_T channel, byte pitch, byte velocity)
{
	if (channel == MIDI_CH_VOCT_A)
	{
		output_dac(0, midi_to_data(pitch));
		trigger_A();
		VEL_A_DUTY = velocity << 1;
	}
	
	if (channel == MIDI_CH_VOCT_B)
	{
		output_dac(1, midi_to_data(pitch));
		trigger_B();
		
		if (CCS_MODE) // don't apply VelB when in KCS mode
		{
			VEL_B_DUTY = velocity << 1;
		}
	}
	
	if (channel == MIDI_CH_KBRD_MODE)
	{
		if (!SWITCH_STATE) // We are in keyboard-controlled sequencer mode
		{
			uint8_t dfam_step = midi_note_to_step(pitch);
			if (dfam_step) {
				int steps_left = steps_between(CUR_DFAM_STEP, dfam_step) + 1;

				// TODO: where should I send the velocity?
				// send it out on Vel. B? I will have to check mode during note on to only
				// apply Vel. B when not in KCS mode.
				
				VEL_B_DUTY = velocity << 1;

				// advance the DFAM's sequencer and then trigger the step
				advance_clock(steps_left);
				CUR_DFAM_STEP = dfam_step;
			}
		}		
	}
}

/*
	cb_Start - handles beginning a sequence
*/
void handleStart()
{
	if (SWITCH_STATE)
	{
		uint8_t steps_left = steps_between(CUR_DFAM_STEP, 1);
		advance_clock(steps_left);
		FOLLOW_MIDI_CLOCK = true;
		CLOCK_COUNT = 0;
		CUR_DFAM_STEP = 0;
	}
}

void handleStop()
{
	FOLLOW_MIDI_CLOCK = false;
}

/*
	cb_Clock - 
*/
void handleClock()
{
	if (FOLLOW_MIDI_CLOCK && SWITCH_STATE)
	{
		// only count clock pulses while sequence is playing and CCS mode is selected
		CLOCK_COUNT = CLOCK_COUNT % (PPQN / CLOCK_DIV) + 1;

		if (CLOCK_COUNT == 1) // we have a new step
		{
			CUR_DFAM_STEP = CUR_DFAM_STEP % NUM_STEPS + 1;
			advance_clock();
		}
	}
}

/*
	cb_Continue - 
*/
void handleContinue()
{   
	FOLLOW_MIDI_CLOCK = true;
}

#endif /* MIDI_EVENTS_H_ */