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

#define MIDI_CH_A 0 // midi channel 1
#define MIDI_CH_B 1 // midi channel 2

/**************************/
/**** Global CONSTANTS ****/
#define NUM_STEPS 8
#define PPQN 24


/**************************/
/**** Global VARIABLES ****/
uint8_t FOLLOW_MIDI_CLOCK = false;
uint8_t CLOCK_COUNT = 0;
uint8_t CUR_DFAM_STEP = 0; // the number of the last DFAM step triggered
uint8_t CLOCK_DIV = 4;
uint8_t PULSES_PER_STEP = PPQN / CLOCK_DIV;
uint8_t MIDI_CHAN_DFAM = 1; // MIDI channel for playing DFAM in "8-voice mono-synth" mode

uint8_t SWITCH_STATE = 0;


#define Channel_T MIDI_NAMESPACE::Channel

/************************************************************************/
/*			  HELPER FUNCTIONS                                          */
/************************************************************************/

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

/************************************************************************/
/*			  EVENT HANDLERS                                            */
/************************************************************************/
void cb_ControlChange(Channel_T channel, byte cc_num, byte cc_val )
{
	switch (cc_num)
	{
		case MIDI_NAMESPACE::ModulationWheel:
			break;
		
		default:
			advance_clock(7);
			break;
	}
}


// TODO: figure out midi channel and do both DACs
void cb_NoteOn(MIDI_NAMESPACE::Channel channel, byte pitch, byte velocity)
{
	
	
	
	
	
	trigger();
	output_dac(0, midi_to_data(pitch));
	
	set_all_LEDs();
}

/*
	cb_Start - handles beginning a sequence
*/
void cb_Start()
{
	clear_all_LEDs();
	bank_B_on();
	
	uint8_t steps_left = steps_between(CUR_DFAM_STEP, 1);
	advance_clock(steps_left);
	FOLLOW_MIDI_CLOCK = true;
	CLOCK_COUNT = 0;
	CUR_DFAM_STEP = 0;
}

void cb_Stop()
{
	clear_all_LEDs();
	bank_B_off();
	
	if (!FOLLOW_MIDI_CLOCK)
	{
		// reset the state of the sequencer
		// if we get a STOP when it is already stopped
		// give the DFAM sequencer a chance to re-sync
		
		//CUR_DFAM_STEP = digitalRead(PIN_SWITCH) ? 0 : 1;
		CUR_DFAM_STEP = 1;
	}

	FOLLOW_MIDI_CLOCK = false;
}

/*
	cb_Clock - 
*/
void cb_Clock()
{
	if (FOLLOW_MIDI_CLOCK && SWITCH_STATE)
	{
		// only count clock pulses while sequence is playing and mode is selected
		CLOCK_COUNT = CLOCK_COUNT % PULSES_PER_STEP + 1;

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
void cb_Continue()
{   
	FOLLOW_MIDI_CLOCK = true;
}

#endif /* MIDI_EVENTS_H_ */