/*
* MidiController.cpp
*
* Created: 7/10/2024 1:00:25 PM
* Author: mikey
*/
#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/atomic.h>

#include "GPIO.h"
#include "MidiController.h"
#include "lib/MIDI.h"

#define SWITCH_DEBOUNCE_DUR 20  // count of Timer1 interrupts b
#define MAX_ADV_LENGTH 1000 // millis
#define MAX_TRIG_LENGTH 100 // millis

#define MCP4822_ABSEL 7
#define MCP4822_IGN 6
#define MCP4822_GAIN 5
#define MCP4822_SHDN 4
#define spi_wait()	while (!(SPI_SPSR & (1 << SPI_SPIF)));

#define MIDI_CH_VOCT_A 1	// channel for v/oct on the primary cv out
#define MIDI_CH_VOCT_B 2	// channel for  v/oct on the secondary cv out (can be the same channel)
#define MIDI_CH_KBRD_MODE 3 // MIDI channel for playing DFAM in "8-voice mono-synth" aka KCS mode

#define KCS_MODE !switch_state
#define CCS_MODE switch_state

#define CC_TRIG_LENGTH MIDI_NAMESPACE::GeneralPurposeController1
#define CC_ADV_WIDTH MIDI_NAMESPACE::GeneralPurposeController2
#define CC_CLOCK_DIV MIDI_NAMESPACE::GeneralPurposeController3

#define NUM_STEPS 8
#define PPQN 24
#define MIDI_ROOT_NOTE 48  // an octave below middle C
#define NUM_DIVISIONS 7
const uint8_t DIVISIONS[NUM_DIVISIONS] = {1, 2, 3, 4, 6, 8, 12};

/************************************************************************/
/*		PUBLIC METHODS		                                            */
/************************************************************************/

/*
	MidiController - default constructor
		initializes the serial transport and MIDI interface
		handles all MIDI events and sequencer logic
*/
MidiController::MidiController():
	transport(),
	cv_out_a(*this, 0),
	cv_out_b(*this, 1),
	keyboard_step_table {48, 50, 52, 53, 55, 57, 59, 60},
	midi((SMT&) transport)
{
	millis_last = 0;
	
	trigger_duration_ms = 1;
	last_sw_read = 0;
	
	adv_clock_ticks = 0;
	
	follow_midi_clock = false;
	clock_count = 0;
	cur_dfam_step = 0; // the number of the last DFAM step triggered
	clock_div = 4;
	switch_state = -1;
	
	time_counter = 0;
}


CvOutput* MidiController::get_cv_out(uint8_t midi_ch)
{
	if (midi_ch == MIDI_CH_VOCT_A)
	{
		return &cv_out_a;
	}
	else if (midi_ch == MIDI_CH_VOCT_B)
	{
		return &cv_out_b;
	}
	else
	{
		return nullptr;
	}
}



void  MidiController::time_inc()
{
	time_counter++;
}

uint32_t MidiController::millis()
{
	cli(); // don't allow ISR to change 32bit _millis while we read it byte by byte
	uint32_t tms = time_counter;
	sei();
	return tms;// / 10;
}

void MidiController::update()
{
	// check the MIDI Tx buffer and deal with any new messages.
	midi.read();

	// update V/oct outputs based on slide and/or vibrato progress
	cv_out_a.check_slide();
	cv_out_b.check_slide();
	
   // read the hardware inputs (the two switches)
   if (millis() - last_sw_read >= SWITCH_DEBOUNCE_DUR)
	{		
		check_mode_switch();
		check_sync_switch();
		last_sw_read = millis();
	}
}


/*
	tx_ready - called to notify the MIDI controller that the USART data register
		is empty and so is ready for a MIDI Tx
*/
void MidiController::tx_ready()
{
	uint8_t midi_byte;
	if (transport.midi_tx_buffer.get(&midi_byte))
	{
		UDR0 = midi_byte;
	}
}

uint8_t MidiController::incoming_message(uint8_t msg)
{
	return transport.midi_rx_buffer.put(msg);
}


/************************************************************************/
/*		HARDWARE OUTPUT                                                 */
/************************************************************************/

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void MidiController::trigger_A()
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
void MidiController::trigger_B()
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
uint16_t MidiController::calculate_ocr_value(uint16_t ms) {
	return (F_CPU / 64) * ms / 1000;
}

/*
	advance_clock - sends a single pulse on the ADV/CLOCK output
*/
void MidiController::advance_clock()
{
	set_bit(ADV_PORT, ADV_OUT);
	
	for (int i = 0; i < adv_clock_ticks; i++)
	{
		/* do nothing, wait a while */
		_NOP();
	}
	
	clear_bit(ADV_PORT, ADV_OUT);	
}

/*
	advance_clock - sends a number of pulses on the ADV/CLOCK output
*/
void MidiController::advance_clock(uint8_t steps)
{
	for (int i = 0; i < steps; i++)
	{
		advance_clock();
	}
}

/*
	output_dac - sends config bits and 12 bits of data to DAC
*/
void MidiController::output_dac(uint8_t channel, uint16_t data)
{
	DAC_CS_PORT &= ~(1<<DAC_CS);	//pull CS low to enable DAC
	
	SPDR = (channel<<MCP4822_ABSEL) | (0<<MCP4822_IGN) | (0<<MCP4822_GAIN) | (1<<MCP4822_SHDN) | ((data>>8) & 0x0F);
	spi_wait();
	
	SPDR = data & 0x00FF;
	spi_wait();
	
	DAC_CS_PORT |= (1<<DAC_CS);		//pull CS high to latch data
}

/*
check_mode_switch -
There are two modes: CCS (clock-controlled sequencer) or KCS (keyboard-controlled
sequencer). SWITCH_STATE is true for CCS, false for KCS.

When the mode switch is flipped we are either switching
- from CCS into KCS (sequence could be in Play or Stop)
- from KCS into CCS
Either way, restart the clock counter and set the DFAM sequencer back to step 1.
*/
void MidiController::check_mode_switch()
{
	uint8_t cur_switch = bit_is_set(MODE_SWITCH_PIN, MODE_SWITCH);
	if (cur_switch == switch_state)
	{
		return;
	}

	switch_state = cur_switch;
	clock_count = 0;
	
	int steps_left = steps_between(cur_dfam_step, 1) + 1;
	advance_clock(steps_left);

	if (switch_state) // switched into "clock-controlled sequencer" mode
	{
		set_bit(LED_BANK_PORT, LED1);
		cur_dfam_step = 0;
	}
	else			  // switched into "keyboard-controlled sequencer" mode
	{
		clear_bit(LED_BANK_PORT, LED1);
		follow_midi_clock = false;
		cur_dfam_step = 1;
	}
}

/*
	check_sync_switch - in case the DFAM sequencer has gotten out of sync
		with CUR_DFAM_STEP, restart it.
*/
void MidiController::check_sync_switch()
{
	if (!bit_is_set(SYNC_BTN_PIN, SYNC_BTN))
	{
		cur_dfam_step = 1;
	}
}



/************************************************************************/
/*		EVENT HANDLERS                                                  */
/************************************************************************/

#define VibratoRate MIDI_NAMESPACE::SoundController7
#define VibratoDepth MIDI_NAMESPACE::SoundController8
#define VibratoDelay MIDI_NAMESPACE::SoundController9

void MidiController::handleCC(byte channel, byte cc_num, byte cc_val )
{
	switch (cc_num)
	{
		case CC_TRIG_LENGTH:
		{
			trigger_duration_ms = (cc_val * MAX_TRIG_LENGTH / 127.0);
			break;
		}
			
		case CC_ADV_WIDTH:
		{
			adv_clock_ticks = (cc_val * MAX_ADV_LENGTH / 127.0);
			break;
		}
			
		case CC_CLOCK_DIV:
		{
			uint8_t div_idx = (int) cc_val * NUM_DIVISIONS / 127.0;
			clock_div = DIVISIONS[div_idx];
			break;
		}
		
		case MIDI_NAMESPACE::PortamentoTime:
		{
			CvOutput* cv_out = get_cv_out(channel);
			if (cv_out != nullptr)
			{
				uint16_t duration = ((uint32_t)cc_val * MAX_SLIDE_LENGTH / ((float) UINT8_MAX));
				cv_out->new_slide_length(duration);
			}
			break;
		}
		
		case VibratoDepth:
		{
			CvOutput* cv_out = get_cv_out(channel);
			if (cv_out != nullptr)
			{
				cv_out->vibrato_depth_cc(cc_val);
			}
			break;
		}

		case VibratoRate:
		{
			CvOutput* cv_out = get_cv_out(channel);
			if (cv_out != nullptr)
			{
				cv_out->vibrato_rate_cc(cc_val);
			}
			break;
		}

		case VibratoDelay:
		{
			CvOutput* cv_out = get_cv_out(channel);
			if (cv_out != nullptr)
			{
				cv_out->vibrato_delay_cc(cc_val);
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

void MidiController::handleNoteOn(byte channel, byte pitch, byte velocity)
{
    CvOutput* cv_out = get_cv_out(channel);
    if (cv_out != nullptr)
	{
		uint8_t send_vel = channel == MIDI_CH_VOCT_A || CCS_MODE;
		cv_out->start_slide(pitch, velocity, send_vel);
	}
	
	if (channel == MIDI_CH_KBRD_MODE)
	{
		if (!switch_state) // We are in keyboard-controlled sequencer mode
		{
			uint8_t dfam_step = midi_note_to_step(pitch);
			if (dfam_step) {
				VEL_B_DUTY = velocity << 1;
				int steps_left = steps_between(cur_dfam_step, dfam_step) + 1;
				advance_clock(steps_left);
				cur_dfam_step = dfam_step;
			}
		}		
	}
}

/*
	handleStart - handles beginning a sequence
*/
void MidiController::handleStart()
{
	if (switch_state)
	{
		uint8_t steps_left = steps_between(cur_dfam_step, 1);
		advance_clock(steps_left);
		follow_midi_clock = true;
		clock_count = 0;
		cur_dfam_step = 0;
	}
}

void MidiController::handleStop()
{
	follow_midi_clock = false;
}

/*
	handleClock - 
*/
void MidiController::handleClock()
{
	if (follow_midi_clock && switch_state)
	{
		// only count clock pulses while sequence is playing and CCS mode is selected
		clock_count = clock_count % (PPQN / clock_div) + 1;

		if (clock_count == 1) // we have a new step
		{
			cur_dfam_step = cur_dfam_step % NUM_STEPS + 1;
			advance_clock();
		}
	}
}

/*
	handleContinue - 
*/
void MidiController::handleContinue()
{   
	follow_midi_clock = true;
}

/*
	handlePitchBend - amt is in the range -8192 to 8191
*/
void MidiController::handlePitchBend(byte midi_ch, int16_t amt)
{
	CvOutput* cv_out = get_cv_out(midi_ch);
	if (cv_out != nullptr)
	{
		cv_out->pitch_bend_event(amt);
	}
}


/************************************************************************/
/*		HELPER METHODS                                                  */
/************************************************************************/

/*
 * steps_between - returns the number of steps the DFAM sequencer would need to 
 *			advance to get from start to end
 */
uint8_t MidiController::steps_between(int start, int end)
{
	if (start == 0 || start == end)
		return NUM_STEPS - 1;

	int steps_left = end - start - 1;
	if (end < start)
		steps_left += NUM_STEPS;
	
	return steps_left;
}

/*
	midi_note_to_step - used in "8-voice monosynth" mode to determine how
		many steps to advance to get to the chosen note
*/
uint8_t MidiController::midi_note_to_step(uint8_t note) {
	for (int i = 0; i < NUM_STEPS; i++) {
		if (note == keyboard_step_table[i])
			return i + 1;
	}
	return false;
}

