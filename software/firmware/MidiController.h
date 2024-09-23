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
#include "CircularBuffer.h"
#include "CvOutput.h"
#include "Serializable.h"

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMidiTransport> MidiInterface;

#define BPM_BUFFER_SIZE 12
#define DFAM_STEPS 8
enum MidiMode { Mono, Poly };

class MctlSettings : public Serializable
{
public:
    MidiMode midi_mode = Mono; /* mono or poly */

    uint8_t midi_ch_A = 1; // channel for v/oct on the primary cv out
    uint8_t midi_ch_B = 2; // channel for v/oct on the secondary cv out (can be same as A)
    uint8_t midi_ch_KCS = 10; // channel for keyboard control
    uint8_t clock_div = 4; /* run the sequence faster/slower relative to midi beat clock */

    uint8_t adv_clock_ticks = 0; /* number of ticks per advance clock pulse */

    uint8_t keyboard_step_table[DFAM_STEPS]; /* val => midi_note number,
												idx => DFAM sequence step to trigger */

    MctlSettings() : keyboard_step_table{48, 50, 52, 53, 55, 57, 59, 60}
    { }

    void serialize(uint8_t* buffer) const override
    {
        size_t offset = 0;

        buffer[offset++] = (MidiMode) midi_mode;
        buffer[offset++] = midi_ch_A;
        buffer[offset++] = midi_ch_B;
        buffer[offset++] = midi_ch_KCS;
        buffer[offset++] = clock_div;
        buffer[offset++] = adv_clock_ticks;

        memcpy(buffer + offset, keyboard_step_table, sizeof(keyboard_step_table));
        offset += sizeof(keyboard_step_table);
    }

    void deserialize(const uint8_t* buffer) override
    {
        size_t offset = 0;

        midi_mode = static_cast<MidiMode>(buffer[offset++]);
        midi_ch_A = buffer[offset++];
        midi_ch_B = buffer[offset++];
        midi_ch_KCS = buffer[offset++];
        clock_div = buffer[offset++];
        adv_clock_ticks = buffer[offset++];

        memcpy(keyboard_step_table, buffer + offset, sizeof(keyboard_step_table));
        offset += sizeof(keyboard_step_table);
    }

    size_t size_bytes() const override
    {
        return sizeof(midi_mode) +
               sizeof(midi_ch_A) +
               sizeof(midi_ch_B) +
               sizeof(midi_ch_KCS) +
               sizeof(clock_div) +
               sizeof(adv_clock_ticks) +
               sizeof(keyboard_step_table);
    }
};

class MidiController
{
	
/***** FIELDS *****/
private:
	MIDI_NAMESPACE::SerialMidiTransport transport;
	
	uint32_t millis_last;
	uint8_t follow_midi_clock;
	uint8_t clock_count;
	uint32_t last_clock;
	uint8_t cur_dfam_step;
	
	volatile uint32_t last_sw_read;
	volatile uint8_t switch_state;
	volatile uint32_t time_counter;
	CircularBuffer<float, BPM_BUFFER_SIZE> clock_period_buffer;

	uint32_t last_kcs_trigger;
	uint8_t kcs_triggered;

public:
	MctlSettings settings;
	CvOutput cv_out_a;
	CvOutput cv_out_b;
	MidiInterface midi;

/***** METHODS *****/
public:
	MidiController();
	float avg_midi_clock_period();
	void init_event_handlers();
	void update();
	uint8_t incoming_message(uint8_t);
	void tx_ready();
	
	// Event handlers
	void handleCC(byte channel, byte cc_num, byte cc_val);
	void handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity);
	void handleNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity);
	void handleStart();
	void handleStop();
	void handleClock();
	void handleContinue();
	void handlePitchBend(uint8_t ch, int amt);
	
	void time_inc();
	uint32_t millis();
	
	void update_midi_channels(uint8_t* channels);
	void update_keyboard_prefs(uint8_t* channels);
	void advance_to_beginning();
	void advance_clock();
	
private:
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
