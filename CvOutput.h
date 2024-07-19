/* 
* CvOutput.h
*
* Created: 7/18/2024 6:49:56 PM
* Author: mikey
*/


#ifndef __CVOUTPUT_H__
#define __CVOUTPUT_H__

#include <avr/io.h>

class CvOutput
{
public:
	volatile uint16_t portamento_time;
	volatile uint8_t is_sliding;
	
	volatile uint32_t slide_start_ms;
	volatile uint16_t slide_length;
	
	volatile uint8_t slide_start_note;
	volatile uint8_t slide_end_note;
	
	uint16_t vib_rate;
	uint16_t vib_depth;
	uint16_t vib_delay;
	

public:
	CvOutput();

};

#endif //__CVOUTPUT_H__
