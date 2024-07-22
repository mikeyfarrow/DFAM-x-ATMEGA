/* 
* NoteCollection.h
*
* Created: 7/21/2024 6:09:15 PM
* Author: mikey
*/


#ifndef __NOTECOLLECTION_H__
#define __NOTECOLLECTION_H__

#include <avr/io.h>

class NoteCollection
{

public:
	
	uint8_t count;
	int16_t notes_held[20]; // -1 for not being held, otherwise midi_note
	

public:
	NoteCollection();
	
	void add_note(uint8_t note, uint32_t time_played);
	void remove_note(uint8_t note);
	uint8_t highest(uint8_t& note);
	uint8_t lowest();
	uint8_t latest();
};

#endif //__NOTECOLLECTION_H__
