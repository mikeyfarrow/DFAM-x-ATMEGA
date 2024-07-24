/* 
* CvOutput.h
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/


#ifndef __CVOUTPUT_H__
#define __CVOUTPUT_H__

#include <avr/io.h>
#include "CircularBuffer.h"

#define LATEST_NOTES_SIZE 20 

class MidiController;

class CvOutput
{
private:
	MidiController& mctl;  // Reference to the MidiController
	
public:
	uint8_t dac_ch;
	
	CircularBuffer<int16_t, LATEST_NOTES_SIZE> latest_notes;
	uint8_t notes_held[100];
	
	uint8_t trigger_duration_ms;
	
	volatile uint16_t portamento_time_asc_user;
	volatile uint16_t portamento_time_desc_user;
	
	volatile uint8_t is_sliding;
	
	volatile uint32_t slide_start_ms;
	volatile uint16_t slide_cur_length;
	
	volatile uint8_t slide_start_note;
	volatile uint8_t slide_end_note;
	
	float vib_tempo_div;
	uint16_t vib_period_ms;
	uint16_t vib_depth_cents;
	uint16_t vib_delay_ms;
	uint8_t vib_tempo_sync; 
	float vibrato_cur_offset; // semitones
	
	float pitch_bend_amt; // -1 to 1
	uint8_t pitch_bend_range; // in semitones
	
	uint32_t last_note_on_ms;

public:
	CvOutput(MidiController& mc, uint8_t dac_channel);
	
	void note_on(uint8_t midi_note, uint8_t velocity, uint8_t send_vel = true, uint8_t add_to_latest = true);
	void slide_progress();
	void pitch_bend(int16_t amt);
	void update_vibrato_offset();
	
	void note_off(uint8_t pitch, uint8_t vel);
	void trigger_A();
	void trigger_B();
	static void output_dac(uint8_t channel, uint16_t data);
	uint16_t midi_to_data(uint8_t midi_note);
	
	uint16_t calculate_ocr_value(uint16_t duration_ms);
	double triangle_wave(double t, double period, bool desc_first = false);
	double sine_wave(double t, double period);
	
	void control_change(uint8_t cc_num, uint8_t cc_val);
   
	int16_t highest();
	int16_t lowest();
	int16_t latest();
};


#endif
