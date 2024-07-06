/* 
* EEPROM_Writer.cpp
*
* Created: 7/5/2024 10:26:44 PM
* Author: mikey
*/


#include "EEPROM_Writer.h"

#include <avr/io.h>
#include <avr/eeprom.h>

EEPROM_Writer::EEPROM_Writer()
{
	idx = 0;
}

void EEPROM_Writer::write_byte(uint8_t ch)
{
	if (idx > E2END)
	{
		idx = 0;
	}
	eeprom_write_byte((uint8_t*)idx, ch);
	idx++;
}

void EEPROM_Writer::delimiter() 
{
	write_byte(0xBB);
	write_byte(0xBB);
}

void EEPROM_Writer::clear_eeprom() 
{	
	uint16_t addr = 0;
	while (addr <= E2END)
	{
		eeprom_write_byte((uint8_t*)addr, 0);
		addr += 1;
	}
}