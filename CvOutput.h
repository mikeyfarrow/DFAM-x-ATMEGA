/* 
* CvOutput.h
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/


#ifndef __CVOUTPUT_H__
#define __CVOUTPUT_H__

#include <avr/io.h>

class MidiController; 

class CvOutput
{
private:
	MidiController& mctl;  // Reference to the MidiController

public:
	uint8_t dac_ch;
	
	volatile uint16_t portamento_time_user;
	volatile uint8_t is_sliding;
	
	volatile uint32_t slide_start_ms;
	volatile uint16_t slide_cur_length;
	
	volatile uint8_t slide_start_note;
	volatile uint8_t slide_end_note;
	
	uint16_t vib_period_ms;
	uint8_t vib_depth_cents;
	uint16_t vib_delay_ms;
	
	float pitch_bend_amt; // -1 to 1
	uint8_t pitch_bend_range; // in semitones
	
	uint32_t last_note_on_ms;

public:
	CvOutput(MidiController& mc, uint8_t dac_channel);
	
	float get_vibrato_semitone_offset(uint32_t ms_now);
	void start_slide(uint8_t midi_note, uint8_t velocity, uint8_t send_vel);
	void check_slide();
	void pitch_bend_event(int16_t amt);
	void new_slide_length(uint16_t dur);
	uint16_t midi_to_data(uint8_t midi_note);
};

#endif
