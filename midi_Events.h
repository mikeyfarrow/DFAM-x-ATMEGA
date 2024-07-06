/*
 * FILE: midi_Events.h
 *
 * This file defines all functions that act as event handlers/callbacks
 * for incoming MIDI messages.
 *
 */

#ifndef MIDI_EVENTS_H_
#define MIDI_EVENTS_H_

#include "./debug_leds.h"
#include "./MCP_4822.h"
#include "./digital_outputs.h"

void cb_NoteOn(byte channel, byte pitch, byte velocity)
{
	send_trigger();
	//set_all_LEDs();
}

void cb_Start()
{
	clear_all_LEDs();
	bank_A_on();
	output_dac(0, 4095>>1);
	output_dac(1, 4095>>1);
}

void cb_Stop()
{
	clear_all_LEDs();
	bank_B_on();
	output_dac(0, 4095);
	output_dac(1, 4095);
}

uint32_t clock_count = 0;

void cb_Clock()
{
	clear_all_LEDs();
}

#endif /* MIDI_EVENTS_H_ */