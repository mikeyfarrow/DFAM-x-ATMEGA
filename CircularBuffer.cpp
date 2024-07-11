/*
* CircularBuffer.cpp
*
* Created: 7/10/2024 1:21:30 PM
* Author: mikey
*/


#include "CircularBuffer.h"

CircularBuffer::CircularBuffer()
{
	write_idx = 0;
	read_idx = 0;
}

/*
	put - adds the item to the buffer and returns 1. If the add fails
		because the buffer is full, then we do not add the item and
		return 0
*/
uint8_t CircularBuffer::put(uint8_t item)
{
	if ((write_idx + 1) % BUFFER_MAX_SIZE == read_idx)
	{
		// buffer is full, avoid overflow
		return 0;
	}
	
	buffer[write_idx] = item;
	write_idx = (write_idx + 1) % BUFFER_MAX_SIZE;
	return 1;
};

/*
	get - if there is a value in the buffer, it is saved to the address
		pointed to by val_ptr and returns 1. If there is no data in the
		buffer, then returns 0.
*/
uint8_t CircularBuffer::get(uint8_t* val_ptr)
{
	if (read_idx == write_idx)
	{
		return 0; // buffer is empty
	}
	
	*val_ptr = buffer[read_idx];
	read_idx = (read_idx + 1) % BUFFER_MAX_SIZE;
	return 1;
};

/*
	ready - returns number of items in the buffer available to be read.
		0 means the buffer is not ready
*/
uint8_t CircularBuffer::ready()
{
	return write_idx - read_idx;
};