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

CvOutput::CvOutput(MidiController& mc, uint8_t ch): mctl(mc), dac_ch(ch)
{
	portamento_time_user = 100;
	is_sliding = false;
	slide_start_ms = UINT32_MAX;
	slide_cur_length = UINT16_MAX;
	
	slide_start_note = UINT8_MAX;
	slide_end_note = UINT8_MAX;
	
	vib_period_ms = 200;
	vib_depth_cents = 50; // magnitude up and down
	vib_delay_ms = 0;
	
	pitch_bend_amt = 0;
	pitch_bend_range = 5;
	
	last_note_on_ms = UINT32_MAX;
}

void CvOutput::new_slide_length(uint16_t dur)
{
	portamento_time_user = dur;
}

float CvOutput::get_vibrato_semitone_offset(uint32_t ms_now)
{
	uint32_t t = (ms_now - last_note_on_ms);
	float coeff = (2*M_PI) / vib_period_ms;
	float mag = sin(coeff*t); // between -1 and 1
	
	return (mag * vib_depth_cents) / 100;
}

void CvOutput::start_slide(uint8_t midi_note, uint8_t velocity, uint8_t send_velocity)
{
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
			
			int16_t inc = ((float) elapsed  * interval) / (float)slide_cur_length;

			mctl.output_dac(dac_ch, start_data + inc);
		}
	}
	else // check for vibrato
	{
		
	}		
}
	
void CvOutput::pitch_bend_event(int16_t amt)
{
	toggle_bit(LED_BANK_PORT, LED2);
	
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
		
		return midi_note * DAC_CAL_VALUE + (pitch_bend_amt * pitch_bend_range * DAC_CAL_VALUE); // add 0.5 ?
	}
}