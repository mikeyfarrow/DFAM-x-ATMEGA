/*
* MidiController.h
*
* Created: 7/10/2024 1:00:25 PM
* Author: mikey

* All MIDI event handlers live inside MidiController as instance methods
*
* From the outside world:
*	When there is a MIDI Rx interrupt:
*		- ask the controller to add a message to the buffer

*	When there is a timer 1ms interrupt:
*		- ask controller to update ticks for trigA, trigB, and debounce
*/

#ifndef __MIDICONTROLLER_H__
#define __MIDICONTROLLER_H__

#include "lib/MIDI.h"
#include "lib/midi_Defs.h"
#include "lib/midi_Namespace.h"

#include "SerialMidiTransport.h"
#include "CvOutput.h"

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMidiTransport> MidiInterface;


class MidiController
{
	
/***** FIELDS *****/
private:
	MIDI_NAMESPACE::SerialMidiTransport transport;
	
	uint32_t millis_last;
	
	volatile uint8_t adv_clock_ticks;
	volatile uint32_t last_sw_read;
	
	uint8_t midi_ch_A; // channel for v/oct on the primary cv out
	uint8_t midi_ch_B; // channel for v/oct on the secondary cv out (can be same as A)
	uint8_t midi_ch_KCS;
	
	uint8_t follow_midi_clock;
	uint8_t clock_count;
	uint32_t last_clock;
	uint8_t cur_dfam_step;
	uint8_t clock_div;
	uint8_t keyboard_step_table[8];
	volatile uint8_t switch_state;
	
	volatile uint32_t time_counter;

public:
	float bpm;
	CvOutput cv_out_a;
	CvOutput cv_out_b;
	MidiInterface midi;

/***** METHODS *****/
public:
	MidiController();
	
	void init_event_handlers();
	void update();
	uint8_t incoming_message(uint8_t); 
	void tx_ready();
	
	// Event handlers
	void handleCC(byte channel, byte cc_num, byte cc_val);
	void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity);
	void handleStart();
	void handleStop();
	void handleClock();
	void handleContinue();
	void handlePitchBend(uint8_t ch, int amt);
	
	void time_inc();
	uint32_t millis();
	
	void update_midi_channels(uint8_t* channels);
	
private:
	void advance_clock();
	void advance_clock(uint8_t steps);
	
	
	void vibrato_depth_cc(uint8_t cc_val);
	
	// update triggers and sequence state
	void check_mode_switch();
	void check_sync_switch();
	
	// Helper methods
	static uint8_t steps_between(int start, int end);
	uint8_t midi_note_to_step(uint8_t note);
};
#endif //__MIDICONTROLLER_H__
