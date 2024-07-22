/* 
* NoteCollection.cpp
*
* Created: 7/21/2024 6:09:15 PM
* Author: mikey
*/


#include <avr/io.h>
#include "NoteCollection.h"

// default constructor
NoteCollection::NoteCollection() : notes_held {-1}
{
	count = 0;
} 


void NoteCollection::add_note(uint8_t note, uint32_t time_played)
{
	if (count == 20)
		return;

	notes_held[count] = note;
	count++;
}

void NoteCollection::remove_note(uint8_t note)
{
	int8_t idx = -1;
	for (int i = 0; i < count; i++)
	{
		if (notes_held[i] == note)
		{
			idx = i;
			break;
		}
	}
	
	if (idx == -1)
		return;
	
	for (int i = idx; i < count - 1; i++)
	{
		notes_held[i] = notes_held[i + i];
	}
	count--;
}

uint8_t NoteCollection::highest(uint8_t& note)
{
	if (count == 0)
		return 0;
	
	uint8_t highest = notes_held[0];
	uint8_t idx = 1;
	while (idx < count)
	{
		if (notes_held[idx] > highest)
			highest = notes_held[idx];
		idx++;
	}
	note = highest;
	return 1;
}

uint8_t NoteCollection::lowest()
{
	
}

uint8_t NoteCollection::latest()
{
	
}