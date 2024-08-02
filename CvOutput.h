/* 
* CvOutput.h
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/


#ifndef __CVOUTPUT_H__
#define __CVOUTPUT_H__

#include <avr/io.h>
#include <string.h>
#include <stddef.h>
#include "CircularBuffer.h"
#include "Serializable.h"

#define LATEST_NOTES_SIZE 20 

#define MIDI_NOTE_MIN 24
#define MIDI_NOTE_MAX 111
#define DAC_CAL_VALUE 47.068966d
#define NUM_CAL_POINTS 8

class MidiController;

enum class TriggerMode { None, Trig, Gate };
enum class RetrigMode  { Off, Highest, Lowest, Latest };
enum class VibratoMode { Off, Free, TempoSync };

class CvSettings : public Serializable
{
public:
	TriggerMode trig_mode;
	RetrigMode retrig_mode;
	uint8_t trigger_duration_ms;

	/* Portamento configuration */
	uint8_t portamento_on;
	uint16_t portamento_time_asc_user;
	uint16_t portamento_time_desc_user;

	/* Vibrato configuration */
	VibratoMode vib_mode;
	uint16_t vib_period_ms;
	uint16_t vib_depth_cents;
	uint16_t vib_delay_ms;
	float vib_tempo_div;

	/* Pitch Bend configuration */
	uint8_t pitch_bend_range;

	/* DAC V/oct digital calibration table: one calibration per C in each octave */
	float calibration_points[NUM_CAL_POINTS];

	CvSettings() : calibration_points{ DAC_CAL_VALUE, DAC_CAL_VALUE, DAC_CAL_VALUE - 0.22, DAC_CAL_VALUE - 0.235,
									   DAC_CAL_VALUE - 0.21, DAC_CAL_VALUE - 0.15, DAC_CAL_VALUE - 0.15, DAC_CAL_VALUE - 0.15 }
	{
		trig_mode = TriggerMode::Trig;
		retrig_mode = RetrigMode::Off;
		trigger_duration_ms = 1;
		pitch_bend_range = 2;
		portamento_on = false;
		portamento_time_asc_user = 0;
		portamento_time_desc_user = 0;
		vib_mode = VibratoMode::Off;
		vib_tempo_div = 1.0;
		vib_period_ms = 200;		// TODO MIN AND MAX??
		vib_depth_cents = 0;
		vib_delay_ms = 0;
	}

	void serialize(uint8_t* buffer) const override
	{
		size_t offset = 0;

		buffer[offset++] = (uint8_t) trig_mode;
		buffer[offset++] = (uint8_t) retrig_mode;
		buffer[offset++] = trigger_duration_ms;
		buffer[offset++] = portamento_on;

		memcpy(buffer + offset, &portamento_time_asc_user, sizeof(portamento_time_asc_user));
		offset += sizeof(portamento_time_asc_user);

		memcpy(buffer + offset, &portamento_time_desc_user, sizeof(portamento_time_desc_user));
		offset += sizeof(portamento_time_desc_user);

		buffer[offset++] = (uint8_t) vib_mode;

		memcpy(buffer + offset, &vib_period_ms, sizeof(vib_period_ms));
		offset += sizeof(vib_period_ms);

		memcpy(buffer + offset, &vib_depth_cents, sizeof(vib_depth_cents));
		offset += sizeof(vib_depth_cents);

		memcpy(buffer + offset, &vib_delay_ms, sizeof(vib_delay_ms));
		offset += sizeof(vib_delay_ms);

		memcpy(buffer + offset, &vib_tempo_div, sizeof(vib_tempo_div));
		offset += sizeof(vib_tempo_div);

		buffer[offset++] = pitch_bend_range;

		memcpy(buffer + offset, calibration_points, sizeof(calibration_points));
		offset += sizeof(calibration_points);
	}

	void deserialize(const uint8_t* buffer) override
	{
		size_t offset = 0;

		trig_mode = static_cast<TriggerMode>(buffer[offset++]);
		retrig_mode = static_cast<RetrigMode>(buffer[offset++]);
		trigger_duration_ms = buffer[offset++];
		portamento_on = buffer[offset++];

		memcpy(&portamento_time_asc_user, buffer + offset, sizeof(portamento_time_asc_user));
		offset += sizeof(portamento_time_asc_user);

		memcpy(&portamento_time_desc_user, buffer + offset, sizeof(portamento_time_desc_user));
		offset += sizeof(portamento_time_desc_user);

		vib_mode = (VibratoMode) buffer[offset++];

		memcpy(&vib_period_ms, buffer + offset, sizeof(vib_period_ms));
		offset += sizeof(vib_period_ms);

		memcpy(&vib_depth_cents, buffer + offset, sizeof(vib_depth_cents));
		offset += sizeof(vib_depth_cents);

		memcpy(&vib_delay_ms, buffer + offset, sizeof(vib_delay_ms));
		offset += sizeof(vib_delay_ms);

		memcpy(&vib_tempo_div, buffer + offset, sizeof(vib_tempo_div));
		offset += sizeof(vib_tempo_div);

		pitch_bend_range = buffer[offset++];

		memcpy(calibration_points, buffer + offset, sizeof(calibration_points));
		offset += sizeof(calibration_points);
	}

	size_t size_bytes() const override
	{
		return	sizeof(trig_mode) +
				sizeof(retrig_mode) +
				sizeof(trigger_duration_ms) +
				sizeof(portamento_on) +
				sizeof(portamento_time_asc_user) +
				sizeof(portamento_time_desc_user) +
				sizeof(vib_mode) +
				sizeof(vib_period_ms) +
				sizeof(vib_depth_cents) +
				sizeof(vib_delay_ms) +
				sizeof(vib_tempo_div) +
				sizeof(pitch_bend_range) +
				sizeof(calibration_points);
	}
};

class CvOutput
{
private:
	MidiController& mctl;  // Reference to the MidiController
	
public:
	CvSettings settings;
	
	uint8_t dac_ch;
	
	uint8_t notes_held[100];
	CircularBuffer<int16_t, LATEST_NOTES_SIZE> latest_notes;
	
	/* state to keep track of slide progress */
	volatile uint8_t is_sliding;
	volatile uint32_t slide_start_ms;
	volatile uint16_t slide_cur_length;
	volatile uint8_t slide_start_note;
	volatile uint8_t slide_end_note;
	
	/* how many semitones of vibrato are currently applied */
	float vibrato_cur_offset;
	
	/* scale factor for current amount of pitch bend (-1 to 1) */
	float pitch_bend_amt;
	
	uint32_t last_note_on_ms;

public:
	CvOutput(MidiController& mc, uint8_t dac_channel);
	void note_on(uint8_t midi_note, uint8_t velocity, uint8_t send_vel = true, uint8_t add_to_latest = true);
	void note_off(uint8_t pitch, uint8_t vel);
	void control_change(uint8_t cc_num, uint8_t cc_val);
	
	void slide_progress();
	void pitch_bend(int16_t amt);
	void update_vibrato_offset();
	double triangle_wave(double t, double period, bool desc_first = false);
	double sine_wave(double t, double period);
	
	void trigger_A();
	void trigger_B();
	
	static void output_dac(uint8_t channel, uint16_t data);
	uint16_t midi_to_data(uint8_t midi_note);
	
	uint16_t calculate_ocr_value(uint16_t duration_ms);
	
	void all_notes_off();
	
	double linear_interpolation(double xValues[], double yValues[], int numValues, double pointX);
	double interpolate_calibration_value(uint8_t note);
	
	int16_t highest();
	int16_t lowest();
	int16_t latest();
};


#endif
