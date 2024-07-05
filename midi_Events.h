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

void cb_NoteOn(byte channel, byte pitch, byte velocity)
{
	set_all_LEDs();
}

void cb_Start()
{
	clear_all_LEDs();
	bank_A_on();
}

void cb_Stop()
{
	clear_all_LEDs();
	bank_B_on();
}

void cb_Clock()
{
	clear_all_LEDs();
}

#endif /* MIDI_EVENTS_H_ */