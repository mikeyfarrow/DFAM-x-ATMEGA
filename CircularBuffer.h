/* 
* CircularBuffer.h
*
* Created: 7/10/2024 1:21:30 PM
* Author: mikey
*/


#ifndef __CIRCULARBUFFER_H__
#define __CIRCULARBUFFER_H__

#include <avr/io.h>

#define BUFFER_MAX_SIZE 100

class CircularBuffer
{
	private:
	uint8_t buffer[BUFFER_MAX_SIZE];
	uint8_t write_idx;
	uint8_t read_idx;
	
	public:
	CircularBuffer();
	uint8_t put(uint8_t item);
	uint8_t get(uint8_t* val_ptr);
	uint8_t ready();
};

#endif //__CIRCULARBUFFER_H__
