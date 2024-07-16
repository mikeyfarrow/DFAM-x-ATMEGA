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

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMidiTransport> MidiInterface;
#define MAX_SLIDE_LENGTH 500 // 500 ms

class MidiController
{
	
/***** FIELDS *****/
private:
	MIDI_NAMESPACE::SerialMidiTransport transport;
	
	volatile uint8_t is_sliding_ch[2];
	volatile uint16_t slide_step_ch[2];
	volatile uint16_t slide_length;
	volatile uint16_t slide_step_max_ch[2];
	volatile uint8_t slide_start_note_ch[2];
	volatile uint8_t slide_end_note_ch[2];
	
	volatile uint8_t trig_A_ticks;
	volatile uint8_t trig_B_ticks;
	volatile uint8_t adv_clock_ticks;
	volatile uint8_t debounce_ticks;
	
	uint8_t trigger_duration_ms;
	uint8_t follow_midi_clock;
	uint8_t clock_count;
	uint8_t cur_dfam_step;
	uint8_t clock_div;
	uint8_t keyboard_step_table[8];
	volatile uint8_t switch_state;

public:
	MidiInterface midi;

/***** METHODS *****/
public:
	MidiController();
	void init_event_handlers();
	void check_for_MIDI();
	void timer_tick();
	uint8_t incoming_message(uint8_t); 
	void tx_ready();
	
	// Event handlers
	void handleCC(byte channel, byte cc_num, byte cc_val);
	void handleNoteOn(byte channel, byte pitch, byte velocity);
	void handleStart();
	void handleStop();
	void handleClock();
	void handleContinue();
	
private:
	void trigger_A();
	void trigger_B();
	void advance_clock();
	void advance_clock(uint8_t steps);
	
	static void output_dac(uint8_t channel, uint16_t data);
	static uint16_t midi_to_data(uint8_t midi_note);
	
	// update triggers and sequence state
	void check_mode_switch();
	void check_sync_switch();
	void check_slide(uint8_t ch);
	void start_slide(byte ch, byte midi_note, byte velocity);
	
	// Helper methods
	static uint8_t steps_between(int start, int end);
	uint8_t midi_note_to_step(uint8_t note);
};
#endif //__MIDICONTROLLER_H__
