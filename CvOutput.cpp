/* 
* CvOutput.cpp
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/

#include <math.h>
#include <util/atomic.h>

#include "CvOutput.h"
#include "GPIO.h"
#include "MidiController.h"

#define MAX_TRIG_LENGTH 100 // millis

#define MCP4822_ABSEL 7
#define MCP4822_IGN 6
#define MCP4822_GAIN 5
#define MCP4822_SHDN 4
#define spi_wait()	while (!(SPI_SPSR & (1 << SPI_SPIF)));

#define MIDI_NOTE_MIN 24
#define MIDI_NOTE_MAX 111
#define DAC_CAL_VALUE 47.068966d

#define PITCH_BEND_MAX 12 // semitones

#define VIB_DELAY_MAX 2000 // ms
#define VIB_DEPTH_MAX 2000  // cents
#define VIB_FREQ_MIN 0.5 // Hz --> 2000 ms period
#define VIB_FREQ_MAX 10 // Hz --> 100 ms period

CvOutput::CvOutput(MidiController& mc, uint8_t ch): mctl(mc), dac_ch(ch) //, notes()
{
	trigger_duration_ms = 1;
	
	portamento_time_user = 0;//100;
	is_sliding = false;
	slide_start_ms = UINT32_MAX;
	slide_cur_length = UINT16_MAX;
	
	slide_start_note = UINT8_MAX;
	slide_end_note = UINT8_MAX;
	
	vib_period_ms = 0;//200;
	vib_depth_cents = 0;//25; // magnitude up and down
	vib_delay_ms = 0;//200;
	vibrato_cur_offset = 0;
	
	pitch_bend_amt = 0;
	pitch_bend_range = 2;
	
	last_note_on_ms = UINT32_MAX;
}


void CvOutput::update_dac_vibrato()
{
	uint32_t elapsed = mctl.millis() - last_note_on_ms;
	if (elapsed < vib_delay_ms || vib_depth_cents == 0)
	{
		return;
	}
	
	elapsed -= vib_delay_ms;
	
	double scale_factor = triangle_wave(elapsed, vib_period_ms);
	vibrato_cur_offset = scale_factor * vib_depth_cents / 100;
	
	output_dac(dac_ch, midi_to_data(slide_end_note));
}

/*
	sine_wave - sine wave, values between -1 and 1 used to create vibrato effect
*/
double CvOutput::sine_wave(double t, double period)
{
	const float coeff = (2*M_PI) / period;
	return  sin(coeff * t); // between -1 and 1
}

/*
	triangle_wave - triangle wave, values between -1 and 1 used to create vibrato effect
*/
double CvOutput::triangle_wave(double t, double period, bool descend_first) {
	if (descend_first)
	{
		t -= (vib_period_ms / 4.0);
	}
	else
	{
		t += (vib_period_ms / 4.0);
	}
	
	double wrapped_time = t - period * floor(t / period);
	double phase = wrapped_time / (period / 2.0);
	int phase_int = (int) phase;
	double fractional_phase = phase - phase_int;

	if (phase_int % 2 == 0) {
		return 2 * fractional_phase - 1; // ascending part of the triangular wave
		} else {
		return 1 - 2 * fractional_phase; // descending part of the triangular wave
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
		output_dac(dac_ch, midi_to_data(slide_end_note));
	}
	else
	{
		check_slide();
	}
	
	// write the velocity and send the trigger
	if (dac_ch == 0)
	{
		VEL_A_DUTY = velocity * 0xFF / 0x7F;
		trigger_A();
	}
	else
	{
		if (send_velocity)
		{
			VEL_B_DUTY = velocity * 0xFF / 0x7F;
		}
		trigger_B();
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
			
			output_dac(dac_ch, midi_to_data(slide_end_note));
		}
		else
		{
			// the slide is ongoing
			uint16_t start_data = midi_to_data(slide_start_note);
			uint16_t end_data = midi_to_data(slide_end_note);
			int16_t interval = end_data - start_data;
			
			int16_t inc = ((float) elapsed  * interval) / (float) slide_cur_length;

			output_dac(dac_ch, start_data + inc);
		}
	}
	else // check for vibrato
	{
		update_dac_vibrato();
	}		
}


/*
	pitch_bend_event - amt is in the range -8192 to 8191
*/
void CvOutput::pitch_bend(int16_t amt)
{
	// get it into the range -1 to 1
	pitch_bend_amt = (((float) amt + 8192) / 16383) * 2 - 1;

	output_dac(dac_ch, midi_to_data(slide_end_note));
}

/*
	midi_to_data - converts a MIDI note into the data bits used by the DAC
		midi_note 0 -> C-1
*/
uint16_t CvOutput::midi_to_data(uint8_t midi_note)
{
	if (midi_note < MIDI_NOTE_MIN)
	   midi_note = MIDI_NOTE_MIN;
	if (midi_note > MIDI_NOTE_MAX)
	   midi_note = MIDI_NOTE_MAX;
	
	uint16_t base_note = (midi_note - MIDI_NOTE_MIN) * DAC_CAL_VALUE;
	int16_t pb_offset = pitch_bend_amt * pitch_bend_range * DAC_CAL_VALUE;
	int16_t vib_offset = vibrato_cur_offset * DAC_CAL_VALUE;
	
	return base_note + pb_offset + vib_offset;
}

/*
	output_dac - sends config bits and 12 bits of data to DAC
*/
void CvOutput::output_dac(uint8_t channel, uint16_t data)
{
	DAC_CS_PORT &= ~(1<<DAC_CS);	//pull CS low to enable DAC
	
	SPDR = (channel<<MCP4822_ABSEL) | (0<<MCP4822_IGN) | (0<<MCP4822_GAIN) | (1<<MCP4822_SHDN) | ((data>>8) & 0x0F);
	spi_wait();
	
	SPDR = data & 0x00FF;
	spi_wait();
	
	DAC_CS_PORT |= (1<<DAC_CS);		//pull CS high to latch data
}

void CvOutput::note_off(uint8_t midi_note, uint8_t vel)
{
	//notes.remove_note(midi_note);
}

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void CvOutput::trigger_A()
{
	set_bit(TRIG_PORT, TRIG_A_OUT);
	
	// Set the compare value for the specified duration in the future
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		OCR1A = TCNT1 + calculate_ocr_value(trigger_duration_ms);
		ENABLE_OCI1A();
	}
}

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void CvOutput::trigger_B()
{
	set_bit(TRIG_PORT, TRIG_B_OUT);
	
	// Set the compare value for the specified duration in the future
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		OCR1B = TCNT1 + calculate_ocr_value(trigger_duration_ms);
		ENABLE_OCI1B();
	}
}

// Calculate OCR value for a given duration in milliseconds
uint16_t CvOutput::calculate_ocr_value(uint16_t ms) {
	return (F_CPU / 64) * ms / 1000;
}

#define CC_PortamentoTime MIDI_NAMESPACE::PortamentoTime
#define CC_TrigLength	  MIDI_NAMESPACE::GeneralPurposeController1
#define CC_PitchBendRange MIDI_NAMESPACE::GeneralPurposeController2
#define CC_VibratoRate	  MIDI_NAMESPACE::SoundController7
#define CC_VibratoDepth	  MIDI_NAMESPACE::SoundController8
#define CC_VibratoDelay   MIDI_NAMESPACE::SoundController9

void CvOutput::control_change(uint8_t cc_num, uint8_t cc_val)
{
	switch (cc_num)
	{
		case CC_PortamentoTime:
			portamento_time_user = ((uint32_t)cc_val * MAX_SLIDE_LENGTH / ((float) UINT8_MAX));
			break;
		
		case CC_TrigLength:
			trigger_duration_ms = (cc_val * MAX_TRIG_LENGTH / 127.0);
			break;
		
		case CC_VibratoRate:
			vib_period_ms = 1000 / (cc_val * VIB_FREQ_MAX / 127.0 + VIB_FREQ_MIN);
			break;
			
		case CC_VibratoDepth:
			vib_depth_cents = ((uint32_t)cc_val * MAX_TRIG_LENGTH / 127.0);
			break;
			
		case CC_VibratoDelay:
			vib_delay_ms = ((uint32_t)cc_val * VIB_DELAY_MAX) / 127.0;
			break;
			
		case CC_PitchBendRange:
			pitch_bend_range = ((uint32_t)cc_val * PITCH_BEND_MAX) / 127.0;
		
		case MIDI_NAMESPACE::RPNMSB: break;
		case MIDI_NAMESPACE::RPNLSB: break;
		case MIDI_NAMESPACE::DataEntryMSB: break;
		case MIDI_NAMESPACE::DataEntryLSB: break;
		case MIDI_NAMESPACE::DataIncrement: break;
		case MIDI_NAMESPACE::DataDecrement: break;
			
		default: break;
	}
}
