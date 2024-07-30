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


#define MCP4822_ABSEL 7
#define MCP4822_IGN 6
#define MCP4822_GAIN 5
#define MCP4822_SHDN 4
#define spi_wait()	while (!(SPI_SPSR & (1 << SPI_SPIF)));


#define MAX_SLIDE_LENGTH 2000 // 500 ms?
#define MAX_TRIG_LENGTH 100 // millis?
#define PITCH_BEND_MAX 12 // semitones

#define VIB_DELAY_MAX 2000 // ms
#define VIB_DEPTH_MAX 800  // cents
#define VIB_FREQ_MIN 0.5 // Hz --> 2000 ms period
#define VIB_FREQ_MAX 10 // Hz --> 100 ms period

float tempo_sync_divisions[8] = {
	4.0, // whole
	2.0, // half
	1.0, // quarter
	0.75, // dotted quarter
	0.5, // eighth
	0.375, // dotted eighth
	0.25, // sixteenth
	0.125, // 32nd
};

RetrigMode retrig_modes[4] = { Off, Highest, Lowest, Latest };

CvOutput::CvOutput(MidiController& mc, uint8_t ch): mctl(mc), dac_ch(ch), notes_held {}, latest_notes()
{
	trig_mode = Trig;
	retrig_mode = Off;
	trigger_duration_ms = 1;
	
	portamento_time_asc_user = 0;//100;
	portamento_time_desc_user = 0;//100;
	
	is_sliding = false;
	slide_start_ms = UINT32_MAX;
	
	slide_cur_length = UINT16_MAX;
	
	slide_start_note = UINT8_MAX;
	slide_end_note = UINT8_MAX;
	
	vib_tempo_div = 1.0;
	vib_period_ms = 200;//200;
	vib_depth_cents = 0;//25; // magnitude up and down
	vib_delay_ms = 0;//200;
	vib_tempo_sync = false;
	vibrato_cur_offset = 0;
	
	pitch_bend_amt = 0;
	pitch_bend_range = 2;
	
	last_note_on_ms = UINT32_MAX;
}


void CvOutput::update_vibrato_offset()
{
	uint32_t elapsed = mctl.millis() - last_note_on_ms;
	if (elapsed < vib_delay_ms || vib_depth_cents == 0)
	{
		return;
	}
	
	elapsed -= vib_delay_ms;
	
	uint16_t period_ms = vib_tempo_sync
		? vib_tempo_div * round(mctl.avg_midi_clock_period()) * 24
		: vib_period_ms;
		
	//double scale_factor = sine_wave(elapsed, period_ms);
	double scale_factor = triangle_wave(elapsed, period_ms);
	vibrato_cur_offset = scale_factor * vib_depth_cents / 100;
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
	t += (descend_first ? -1 : 1) * (vib_period_ms / 4.0);

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
							? portamento_time_desc_user
							: portamento_time_asc_user;
	slide_start_ms = mctl.millis();
	is_sliding = slide_cur_length > 0 && slide_start_note != slide_end_note;
	
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
		if (trig_mode == Trig)
		{   // the MidiController handles triggers in Poly mode
			trigger_A();
		}
		else if (trig_mode == Gate)
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

		if (trig_mode == Trig)
		{   // the MidiController handles triggers in Poly mode
			trigger_B();
		}
		else if (trig_mode == Gate)
		{
			set_bit(TRIG_PORT, TRIG_B_OUT);
		}
	}
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

#define DAC_MIN 0x00
#define DAC_MAX 0x0FFF

/*
	midi_to_data - converts a MIDI note into the data bits used by the DAC
		midi_note 0 -> C-1
*/
uint16_t CvOutput::midi_to_data(uint8_t midi_note)
{
	midi_note = (uint8_t) in_range(midi_note, MIDI_NOTE_MIN, MIDI_NOTE_MAX);
	
	int32_t base_note = (midi_note - MIDI_NOTE_MIN) * DAC_CAL_VALUE;
	int32_t pb_offset = pitch_bend_amt * pitch_bend_range * DAC_CAL_VALUE;
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

void CvOutput::note_off(uint8_t midi_note, uint8_t vel)
{
	notes_held[midi_note] = 0;
	
	int16_t note;
	switch (retrig_mode)
	{
		case Highest:	note = highest(); break;
		case Lowest:	note = lowest(); break;
		case Latest:	note = latest(); break;
		default:		note = latest(); break;
	}
	
	if (note == -1 && trig_mode == Gate)
	{
		clear_bit(TRIG_PORT, dac_ch ? TRIG_B_OUT : TRIG_A_OUT);
	}
	
	if (note > -1 && retrig_mode != Off)
	{
		note_on(note, notes_held[note], false, false);
	}
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
		OCR1A = TCNT1 + calculate_ocr_value(trigger_duration_ms);
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
		OCR1B = TCNT1 + calculate_ocr_value(trigger_duration_ms);
		ENABLE_OCI1B();
	}
}

// Calculate OCR value for a given duration in milliseconds
uint16_t CvOutput::calculate_ocr_value(uint16_t ms) {
	return (F_CPU / 64) * ms / 1000;
}

#define CC_TrigLength	  MIDI_NAMESPACE::GeneralPurposeController1 // 16
#define CC_PitchBendRange MIDI_NAMESPACE::GeneralPurposeController2 // 17

#define CC_PortamentoTime     MIDI_NAMESPACE::PortamentoTime // cc# 5
#define CC_PortamentoTimeDesc MIDI_NAMESPACE::GeneralPurposeController3 // cc# 18
#define CC_PortamentoTimeAsc  MIDI_NAMESPACE::GeneralPurposeController4 // cc# 19

#define CC_RetrigMode  MIDI_NAMESPACE::GeneralPurposeController5 // cc# 80

#define CC_VibratoRate	  MIDI_NAMESPACE::SoundController7 // 76
#define CC_VibratoDepth	  MIDI_NAMESPACE::SoundController8
#define CC_VibratoDelay   MIDI_NAMESPACE::SoundController9
#define CC_VibratoSync    MIDI_NAMESPACE::SoundController10 // 79. Value: on or off

/*
    AllSoundOff                 = 120,
    ResetAllControllers         = 121,
    LocalControl                = 122,
    AllNotesOff                 = 123,
    OmniModeOff                 = 124,
    OmniModeOn                  = 125,
    MonoModeOn                  = 126,
    PolyModeOn                  = 127
*/
void CvOutput::control_change(uint8_t cc_num, uint8_t cc_val)
{
	uint16_t time;
	switch (cc_num)
	{
		case CC_PortamentoTime:
			time = ((uint32_t)cc_val * MAX_SLIDE_LENGTH) / ((float) UINT8_MAX);
			portamento_time_desc_user = time;
			portamento_time_asc_user = time;
			break;
			
		case CC_PortamentoTimeDesc:
			portamento_time_desc_user = ((uint32_t)cc_val * MAX_SLIDE_LENGTH / ((float) UINT8_MAX));
			break;
		
		case CC_PortamentoTimeAsc:
			portamento_time_asc_user = ((uint32_t)cc_val * MAX_SLIDE_LENGTH / ((float) UINT8_MAX));
			break;
		
		case CC_TrigLength:
			trigger_duration_ms = (cc_val * MAX_TRIG_LENGTH / 127.0);
			if (trigger_duration_ms < 1) trigger_duration_ms = 1;
			break;
		
		case CC_VibratoRate:
			vib_tempo_div = tempo_sync_divisions[cc_val / 16];
			vib_period_ms = 1000 / (cc_val * VIB_FREQ_MAX / 127.0 + VIB_FREQ_MIN);
			break;
			
		case CC_VibratoDepth:
			vib_depth_cents = ((uint32_t)cc_val * VIB_DEPTH_MAX) / 127.0;
			break;
			
		case CC_VibratoDelay:
			vib_delay_ms = ((uint32_t)cc_val * VIB_DELAY_MAX) / 127.0;
			break;
			
		case CC_VibratoSync:
			vib_tempo_sync = cc_val > 63; // more than half => ON
			break;
			
		case CC_PitchBendRange:
			pitch_bend_range = ((uint32_t)cc_val * PITCH_BEND_MAX) / 127.0;
		
		case CC_RetrigMode:
			retrig_mode = retrig_modes[cc_val/32];
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