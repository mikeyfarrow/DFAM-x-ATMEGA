/*
* MidiController.cpp
*
* Created: 7/10/2024 1:00:25 PM
* Author: mikey
*/
#include <avr/io.h>
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
	cv_out_a(*this, 0),
	cv_out_b(*this, 1),
	keyboard_step_table {48, 50, 52, 53, 55, 57, 59, 60},
	midi((SMT&) transport)
{
	midi_ch_A = 1;
	midi_ch_B = 2;
	midi_ch_KCS = 10;
	
	millis_last = 0;
	last_sw_read = 0;
	adv_clock_ticks = 0;
	
	follow_midi_clock = false;
	clock_count = 0;
	cur_dfam_step = 0; // the number of the last DFAM step triggered
	clock_div = 4;
	switch_state = -1;
	
	time_counter = 0;
}

void MidiController::update_midi_channels(uint8_t* ch)
{
	midi_ch_A = ch[0];
	midi_ch_B = ch[1];
	midi_ch_KCS = ch[2];
}

void MidiController::time_inc()
{
	time_counter++;
}

uint32_t MidiController::millis()
{
	cli(); // don't allow ISR to change 32bit _millis while we read it byte by byte
	uint32_t tms = time_counter;
	sei();
	return tms;// / 10;
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


/*
	tx_ready - called to notify the MIDI controller that the USART data register
		is empty and so is ready for a MIDI Tx
*/
void MidiController::tx_ready()
{
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
	
	for (int i = 0; i < adv_clock_ticks; i++)
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
		set_bit(LED_BANK_PORT, LED1);
		cur_dfam_step = 0;
	}
	else			  // switched into "keyboard-controlled sequencer" mode
	{
		clear_bit(LED_BANK_PORT, LED1);
		follow_midi_clock = false;
		cur_dfam_step = 1;
	}
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

#define CC_AdvClockWidth  MIDI_NAMESPACE::GeneralPurposeController2
#define CC_ClockDiv		  MIDI_NAMESPACE::GeneralPurposeController3

void MidiController::handleCC(byte channel, byte cc_num, byte cc_val )
{
	if (channel == midi_ch_A)
		cv_out_a.control_change(cc_num, cc_val);
	
	if (channel == midi_ch_B)
		cv_out_b.control_change(cc_num, cc_val);

	/* Considering these CCs as "Global" for now ... */
	switch (cc_num)
	{
		case CC_AdvClockWidth:
		{
			adv_clock_ticks = (cc_val * MAX_ADV_LENGTH / 127.0);
			break;
		}
			
		case CC_ClockDiv:
		{
			uint8_t div_idx = (int) cc_val * NUM_DIVISIONS / 127.0;
			clock_div = DIVISIONS[div_idx];
			break;
		}
		
		default:
		{
			break;
		}
	}
}

void MidiController::handleNoteOn(uint8_t channel, uint8_t midi_note, uint8_t velocity)
{
	if (channel == midi_ch_A)
		cv_out_a.note_on(midi_note, velocity, true);
	
	if (channel == midi_ch_B) // only send vel. B in CCS mode
		cv_out_b.note_on(midi_note, velocity, CCS_MODE); 

	if (channel == midi_ch_KCS)
	{
		if (KCS_MODE) // We are in keyboard-controlled sequencer mode
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
	if (follow_midi_clock && switch_state)
	{
		// only count clock pulses while sequence is playing and CCS mode is selected
		clock_count = clock_count % (PPQN / clock_div) + 1;

		if (clock_count == 1) // we have a new step
		{
			cur_dfam_step = cur_dfam_step % NUM_STEPS + 1;
			advance_clock();
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
	if (midi_ch == midi_ch_A)
		cv_out_a.pitch_bend(amt);
	
	if (midi_ch == midi_ch_B) // only send vel. B in CCS mode
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
		if (note == keyboard_step_table[i])
			return i + 1;
	}
	return false;
}

