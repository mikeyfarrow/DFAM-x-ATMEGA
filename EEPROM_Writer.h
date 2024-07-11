/*
* EEPROM_Writer.h
*
* Created: 7/5/2024 10:26:45 PM
* Author: mikey
*/


#ifndef __EEPROM_WRITER_H__
#define __EEPROM_WRITER_H__

#include <avr/io.h>

class EEPROM_Writer
{
	private:
	uint16_t idx;

	//functions
	public:
	EEPROM_Writer();
	void write_byte(uint8_t ch);
	void delimiter();
	void clear_eeprom();

}; //EEPROM_Writer

#endif //__EEPROM_WRITER_H__
