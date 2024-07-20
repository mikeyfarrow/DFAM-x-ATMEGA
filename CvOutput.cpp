/* 
* CvOutput.cpp
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/

#include <math.h>

#include "CvOutput.h"
#include "GPIO.h"
#include "MidiController.h"

#define MIDI_NOTE_MIN 12
#define MIDI_NOTE_MAX 111
#define DAC_CAL_VALUE 47.068966d

#define VIB_DELAY_MAX 2000 // ms
#define VIB_DEPTH_MAX 2000  // cents
#define VIB_FREQ_MIN 0.5 // Hz --> 2000 ms period
#define VIB_FREQ_MAX 10 // Hz --> 100 ms period

CvOutput::CvOutput(MidiController& mc, uint8_t ch): mctl(mc), dac_ch(ch)
{
	portamento_time_user = 100;
	is_sliding = false;
	slide_start_ms = UINT32_MAX;
	slide_cur_length = UINT16_MAX;
	
	slide_start_note = UINT8_MAX;
	slide_end_note = UINT8_MAX;
	
	vib_period_ms = 200;
	vib_depth_cents = 25; // magnitude up and down
	vib_delay_ms = 200;
	vibrato_cur_offset = 0;
	
	pitch_bend_amt = 0;
	pitch_bend_range = 2;
	
	last_note_on_ms = UINT32_MAX;
}

void CvOutput::new_slide_length(uint16_t dur)
{
	portamento_time_user = dur;
}

void CvOutput::update_dac_vibrato()
{
	uint32_t elapsed = mctl.millis() - last_note_on_ms;
	if (elapsed < vib_delay_ms || vib_depth_cents == 0)
	{
		return;
	}
	
	elapsed -= vib_delay_ms;
	//float coeff = (2*M_PI) / vib_period_ms;
	//float mag = sin(coeff * elapsed); // between -1 and 1
	//vibrato_cur_offset = (mag * vib_depth_cents) / 100;
	
	double mag = triangle_wave(elapsed, vib_period_ms, 1);
	vibrato_cur_offset = mag * vib_depth_cents / 100;
	
	mctl.output_dac(dac_ch, midi_to_data(slide_end_note));
}



double CvOutput::triangle_wave(double t, double period, double amplitude)
{
	// Normalize t to fit within one period
	int full_periods = (int)(t / period);
	double t_mod = t - full_periods * period;
	if (t_mod < period / 2.0) 
	{
		return (4 * amplitude / period) * t_mod;
	} 
	else 
	{
		return (4 * amplitude / period) * (period - t_mod);
	}
}

void CvOutput::start_slide(uint8_t midi_note, uint8_t velocity, uint8_t send_velocity)
{
	last_note_on_ms = mctl.millis();
	vibrato_cur_offset = 0;
	
	// set the start and end note for the slide
	slide_start_note = slide_end_note;
	slide_end_note = midi_note;
	if (slide_start_note == UINT8_MAX)
	{
		// if this is the first note, there is no slide
		slide_start_note = slide_end_note;
	}
	
	// set the time bounds for the slide and start the slide
	slide_cur_length = portamento_time_user;
	slide_start_ms = mctl.millis();
	is_sliding = portamento_time_user > 0;
	
	if (!is_sliding)
	{
		mctl.output_dac(dac_ch, midi_to_data(slide_end_note));
	}
	else
	{
		check_slide();
	}
	
	// write the velocity and send the trigger
	if (dac_ch == 0)
	{
		VEL_A_DUTY = velocity * 0xFF / 0x7F;
		mctl.trigger_A();
	}
	else
	{
		if (send_velocity)
		{
			VEL_B_DUTY = velocity * 0xFF / 0x7F;
		}
		mctl.trigger_B();
	}
}

void CvOutput::vibrato_depth_cc(uint8_t cc_val)
{
	vib_depth_cents = ((uint32_t)cc_val * VIB_DEPTH_MAX / 127.0);
}

void CvOutput::vibrato_rate_cc(uint8_t cc_val)
{
	float freq = cc_val * VIB_FREQ_MAX / 127.0 + VIB_FREQ_MIN;
	vib_period_ms = 1000 / freq;
}

void CvOutput::vibrato_delay_cc(uint8_t cc_val)
{
	vib_delay_ms = ((uint32_t)cc_val * VIB_DELAY_MAX) / 127.0;
}

void CvOutput::check_slide()
{
	if (is_sliding)
	{
		uint32_t elapsed = mctl.millis() - slide_start_ms;
		if (elapsed >= slide_cur_length)
		{
			// the slide is complete
			is_sliding = false;
			
			mctl.output_dac(dac_ch, midi_to_data(slide_end_note));
		}
		else
		{
			// the slide is ongoing
			uint16_t start_data = midi_to_data(slide_start_note);
			uint16_t end_data = midi_to_data(slide_end_note);
			int16_t interval = end_data - start_data;
			
			int16_t inc = ((float) elapsed  * interval) / (float) slide_cur_length;

			mctl.output_dac(dac_ch, start_data + inc);
		}
	}
	else // check for vibrato
	{
		update_dac_vibrato();
	}		
}

void CvOutput::pitch_bend_event(int16_t amt)
{
	// get it into the range -1 to 1
	pitch_bend_amt = (((float) amt + 8192) / 16383) * 2 - 1;

	mctl.output_dac(dac_ch, midi_to_data(slide_end_note));
}

/*
	midi_to_data - converts a MIDI note into the data bits used by the DAC
		midi_note 0 -> C-1
*/
uint16_t CvOutput::midi_to_data(uint8_t midi_note)
{
	if (midi_note < MIDI_NOTE_MIN || midi_note > MIDI_NOTE_MAX)
	{
		// error?
		return 0;
	}
	else
	{
		uint16_t base_note = (midi_note - 24) * DAC_CAL_VALUE;
		int16_t pb_offset = pitch_bend_amt * pitch_bend_range * DAC_CAL_VALUE;
		int16_t vib_offset = vibrato_cur_offset * DAC_CAL_VALUE;
		return base_note + pb_offset + vib_offset;
		
		//return midi_note * DAC_CAL_VALUE + (pitch_bend_amt * pitch_bend_range * DAC_CAL_VALUE); // add 0.5 ?
	}
}