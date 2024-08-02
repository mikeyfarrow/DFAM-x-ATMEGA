/* 
* CvOutput.cpp
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/

#include <math.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "CvOutput.h"
#include "GPIO.h"
#include "MidiController.h"

#define DAC_MIN 0x00
#define DAC_MAX 0x0FFF

#define MCP4822_ABSEL 7
#define MCP4822_IGN 6
#define MCP4822_GAIN 5
#define MCP4822_SHDN 4
#define spi_wait()	while (!(SPI_SPSR & (1 << SPI_SPIF)));

#define MAX_SLIDE_LENGTH 2000 // 500 ms?
#define MAX_TRIG_LENGTH 50 // millis?
#define PITCH_BEND_MAX 12 // semitones

#define VIB_DELAY_MAX 2000 // ms
#define VIB_DEPTH_MAX 800  // cents
#define VIB_FREQ_MIN 0.5 // Hz --> 2000 ms period
#define VIB_FREQ_MAX 10 // Hz --> 100 ms period

const float tempo_sync_divisions[8] = {
	4.0, // whole
	2.0, // half
	1.0, // quarter
	0.75, // dotted quarter
	0.5, // eighth
	0.375, // dotted eighth
	0.25, // sixteenth
	0.125, // 32nd
};

const RetrigMode retrig_modes[4] = { RetrigMode::Off, RetrigMode::Highest, RetrigMode::Lowest, RetrigMode::Latest };
const TriggerMode trig_modes[3] = { TriggerMode::Trig, TriggerMode::Gate, TriggerMode::None };

CvOutput::CvOutput(MidiController& mc, uint8_t ch):
			mctl(mc),
			dac_ch(ch),
			notes_held {},
			latest_notes()
{
	is_sliding = false;
	slide_start_ms = UINT32_MAX;
	slide_cur_length = UINT16_MAX;
	slide_start_note = UINT8_MAX;
	slide_end_note = UINT8_MAX;
	
	vibrato_cur_offset = 0;
	vib_lfo = HalfWave;
	
	pitch_bend_amt = 0;
	
	last_note_on_ms = UINT32_MAX;
}


void CvOutput::update_vibrato_offset()
{
	uint32_t elapsed = mctl.millis() - last_note_on_ms;
	if (elapsed < settings.vib_delay_ms || settings.vib_depth_cents == 0)
	{
		return;
	}
	
	elapsed -= settings.vib_delay_ms;
	
	uint16_t period_ms = settings.vib_mode == VibratoMode::TempoSync
		? settings.vib_tempo_div * round(mctl.avg_midi_clock_period()) * 24
		: settings.vib_period_ms;
		
	//double scale_factor = sine_wave(elapsed, period_ms);
	double scale_factor = triangle_wave(elapsed, period_ms);
	vibrato_cur_offset = scale_factor * settings.vib_depth_cents / 100;
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
double CvOutput::triangle_wave(double t, double period, bool descend_first)
{
	t += (descend_first ? -1 : 1) * (settings.vib_period_ms / 4.0);

	double wrapped_time = t - period * floor(t / period);
	double phase = wrapped_time / (period / 2.0);
	int phase_int = (int)phase;
	double fractional_phase = phase - phase_int;

	double wave_value;
	if (phase_int % 2 == 0)
	{
		wave_value = 2 * fractional_phase - 1; // ascending part of the triangular wave
	}
	else
	{
		wave_value = 1 - 2 * fractional_phase; // descending part of the triangular wave
	}

	switch (vib_lfo) {
		case Rectified: return fabs(wave_value);
		case HalfWave:	return wave_value < 0 ? 0 : wave_value;
		default:		return wave_value;
	}
}

void CvOutput::note_on(uint8_t midi_note, uint8_t velocity, uint8_t send_velocity, uint8_t add_to_latest)
{
	last_note_on_ms = mctl.millis();
	if (add_to_latest)
	{
		latest_notes.put(midi_note);
		notes_held[midi_note] = velocity;
	}
	
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
	slide_cur_length = slide_start_note > slide_end_note
							? settings.portamento_time_desc_user
							: settings.portamento_time_asc_user;
	slide_start_ms = mctl.millis();
	is_sliding = settings.portamento_on && slide_cur_length > 0 && slide_start_note != slide_end_note;
	
	if (!is_sliding)
	{
		output_dac(dac_ch, midi_to_data(slide_end_note));
	}
	else
	{
		slide_progress();
	}
	
	// write the velocity and send the trigger
	if (dac_ch == 0)
	{
		VEL_A_DUTY = velocity * 0xFF / 0x7F;
		if (settings.trig_mode == TriggerMode::Trig)
		{   // the MidiController handles triggers in Poly mode
			trigger_A();
		}
		else if (settings.trig_mode == TriggerMode::Gate)
		{
			set_bit(TRIG_PORT, TRIG_A_OUT);
		}
	}
	else
	{
		if (send_velocity)
		{
			VEL_B_DUTY = velocity * 0xFF / 0x7F;
		}

		if (settings.trig_mode == TriggerMode::Trig)
		{   // the MidiController handles triggers in Poly mode
			trigger_B();
		}
		else if (settings.trig_mode == TriggerMode::Gate)
		{
			set_bit(TRIG_PORT, TRIG_B_OUT);
		}
	}
}

void CvOutput::note_off(uint8_t midi_note, uint8_t vel)
{
	notes_held[midi_note] = 0;
	
	int16_t note;
	switch (settings.retrig_mode)
	{
		case RetrigMode::Highest:	note = highest(); break;
		case RetrigMode::Lowest:	note = lowest(); break;
		case RetrigMode::Latest:	note = latest(); break;
		default:					note = latest(); break;
	}
	
	if (note == -1 && settings.trig_mode == TriggerMode::Gate)
	{
		clear_bit(TRIG_PORT, dac_ch ? TRIG_B_OUT : TRIG_A_OUT);
	}
	
	if (note > -1 && settings.retrig_mode != RetrigMode::Off)
	{
		note_on(note, notes_held[note], false, false);
	}
}

void CvOutput::all_notes_off()
{
	for (int i = 0; i < 100; i++)
	{
		notes_held[i] = 0;
	}
	
	while (latest_notes.ready())
	{
		int16_t* dummy;
		latest_notes.get(dummy);
	}
	is_sliding = false;
	vibrato_cur_offset = 0;
}

void CvOutput::slide_progress()
{
	update_vibrato_offset();
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
		output_dac(dac_ch, midi_to_data(slide_end_note));
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

int32_t in_range(int32_t val, int32_t min, int32_t max)
{
	if (val > max) val = max;
	if (val < min) val = min;
	return val;
}

/*
	midi_to_data - converts a MIDI note into the data bits used by the DAC
		midi_note 0 -> C-1
*/
uint16_t CvOutput::midi_to_data(uint8_t midi_note)
{
	midi_note = (uint8_t) in_range(midi_note, MIDI_NOTE_MIN, MIDI_NOTE_MAX);
	midi_note -= MIDI_NOTE_MIN;
	
	// int32_t base_note = (midi_note) * DAC_CAL_VALUE;
	double calibration_val = interpolate_calibration_value(midi_note);
	int32_t base_note = midi_note * calibration_val;
	int32_t pb_offset = pitch_bend_amt * settings.pitch_bend_range * DAC_CAL_VALUE;
	int32_t vib_offset = vibrato_cur_offset * DAC_CAL_VALUE;
	
	int32_t dac_data = base_note + pb_offset + vib_offset;
	dac_data = in_range(dac_data, DAC_MIN, DAC_MAX);
	
	return dac_data;
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

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void CvOutput::trigger_A()
{
	TIFR1 |= (1 << OCF1A);
	set_bit(TRIG_PORT, TRIG_A_OUT);
	
	// Set the compare value for the specified duration in the future
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		OCR1A = TCNT1 + calculate_ocr_value(settings.trigger_duration_ms);
		ENABLE_OCI1A();
	}
}

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void CvOutput::trigger_B()
{
	TIFR1 |= (1 << OCF1B);
	set_bit(TRIG_PORT, TRIG_B_OUT);
	
	// Set the compare value for the specified duration in the future
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		OCR1B = TCNT1 + calculate_ocr_value(settings.trigger_duration_ms);
		ENABLE_OCI1B();
	}
}

// Calculate OCR value for a given duration in milliseconds
uint16_t CvOutput::calculate_ocr_value(uint16_t ms) {
	return (F_CPU / 64) * ms / 1000;
}

#define CC_TrigLength			MIDI_NAMESPACE::GeneralPurposeController1 // 16
#define CC_PitchBendRange		MIDI_NAMESPACE::GeneralPurposeController2 // 17
#define CC_PortamentoOnOff		MIDI_NAMESPACE::Portamento			// cc# 65
#define CC_PortamentoTime		MIDI_NAMESPACE::PortamentoTime		// cc# 5
#define CC_PortamentoTimeDesc	MIDI_NAMESPACE::GeneralPurposeController3 // cc# 18
#define CC_PortamentoTimeAsc	MIDI_NAMESPACE::GeneralPurposeController4 // cc# 19
#define CC_RetrigMode			MIDI_NAMESPACE::GeneralPurposeController5 // cc# 80
#define CC_TrigMode				MIDI_NAMESPACE::GeneralPurposeController6
#define CC_VibratoRate			MIDI_NAMESPACE::SoundController7 // 76
#define CC_VibratoDepth			MIDI_NAMESPACE::SoundController8
#define CC_VibratoDelay			MIDI_NAMESPACE::SoundController9
#define CC_VibratoSync			MIDI_NAMESPACE::SoundController10 // 79. Value: on or off

void CvOutput::control_change(uint8_t cc_num, uint8_t cc_val)
{
	uint16_t time;
	switch (cc_num)
	{
		case MIDI_NAMESPACE::Legato:
			settings.trig_mode = cc_val > 63 ? TriggerMode::Gate : TriggerMode::Trig;
			break;
			
		case CC_PortamentoOnOff:
			settings.portamento_on = cc_val > 63;
			break;
			
		case CC_PortamentoTime:
			time = ((uint32_t)cc_val * MAX_SLIDE_LENGTH) / ((float) UINT8_MAX);
			settings.portamento_time_desc_user = time;
			settings.portamento_time_asc_user = time;
			break;
			
		case CC_PortamentoTimeDesc:
			settings.portamento_time_desc_user = ((uint32_t)cc_val * MAX_SLIDE_LENGTH / ((float) UINT8_MAX));
			break;
		
		case CC_PortamentoTimeAsc:
			settings.portamento_time_asc_user = ((uint32_t)cc_val * MAX_SLIDE_LENGTH / ((float) UINT8_MAX));
			break;
		
		case CC_TrigLength:
			settings.trigger_duration_ms = (cc_val * MAX_TRIG_LENGTH / 127.0);
			if (settings.trigger_duration_ms < 1) settings.trigger_duration_ms = 1;
			break;
		
		case CC_VibratoRate:
			settings.vib_tempo_div = tempo_sync_divisions[cc_val / 16];
			settings.vib_period_ms = 1000 / (cc_val * VIB_FREQ_MAX / 127.0 + VIB_FREQ_MIN);
			break;
			
		case CC_VibratoDepth:
			settings.vib_depth_cents = ((uint32_t)cc_val * VIB_DEPTH_MAX) / 127.0;
			break;
			
		case CC_VibratoDelay:
			settings.vib_delay_ms = ((uint32_t)cc_val * VIB_DELAY_MAX) / 127.0;
			break;
			
		case CC_VibratoSync:
			settings.vib_mode = cc_val > 63 ? VibratoMode::TempoSync : VibratoMode::Free;
			break;
			
		case CC_PitchBendRange:
			settings.pitch_bend_range = ((uint32_t)cc_val * PITCH_BEND_MAX) / 127.0;
		
		case CC_RetrigMode:
			settings.retrig_mode = retrig_modes[cc_val/32];
			break;
			
		case CC_TrigMode:
			settings.trig_mode = trig_modes[(int)(cc_val/42.66)];
			break;
		
		case MIDI_NAMESPACE::RPNMSB: break;
		case MIDI_NAMESPACE::RPNLSB: break;
		case MIDI_NAMESPACE::DataEntryMSB: break;
		case MIDI_NAMESPACE::DataEntryLSB: break;
		case MIDI_NAMESPACE::DataIncrement: break;
		case MIDI_NAMESPACE::DataDecrement: break;
			
		default: break;
	}
}

double CvOutput::linear_interpolation(double xValues[], double yValues[], int numValues, double pointX) {
	// Handle out-of-bound points
	if (pointX <= xValues[0])
	   return yValues[0];
	else if (pointX >= xValues[numValues - 1])
	   return yValues[numValues - 1];
	

	// Find the interval [x0, x1] where x0 <= pointX <= x1
	for (int i = 0; i < numValues - 1; i++) {
		if (pointX >= xValues[i] && pointX <= xValues[i + 1]) {
			double x0 = xValues[i];
			double y0 = yValues[i];
			double x1 = xValues[i + 1];
			double y1 = yValues[i + 1];

			// Linear interpolation formula
			double slope = (y1 - y0) / (x1 - x0);
			return y0 + slope * (pointX - x0);
		}
	}

	// If we didn't find the interval, return an error value (shouldn't happen)
	return -1;
}

double CvOutput::interpolate_calibration_value(uint8_t note) {
	double xVals[NUM_CAL_POINTS] = {0, 12, 24, 36, 48, 60, 72, 84};
	double yVals[NUM_CAL_POINTS];
	for (int i = 0; i < NUM_CAL_POINTS; i++) {
		yVals[i] = (double) settings.calibration_points[i];
	}
	return linear_interpolation(xVals, yVals, NUM_CAL_POINTS, (double)note);
}

int16_t CvOutput::highest()
{
	for (int i = 99; i >= 0; i--)
		if (notes_held[i])
			return i;
			
	return -1;
}

int16_t CvOutput::lowest()
{
	for (int i = 0; i < 100; i++)
		if (notes_held[i])
			return i;
			
	return -1;
}

int16_t CvOutput::latest()
{
	uint8_t idx_latest = latest_notes.write_idx - 1;
	for (uint8_t i = 0; i < LATEST_NOTES_SIZE; i++)
	{
		int8_t r = idx_latest - i % LATEST_NOTES_SIZE;
		int8_t note = latest_notes.buffer[r < 0 ? r + LATEST_NOTES_SIZE : r];

		if (notes_held[note]) {
			return note;
		}
	}
	
	return -1;
}