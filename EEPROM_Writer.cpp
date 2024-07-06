/* 
* EEPROM_Writer.cpp
*
* Created: 7/5/2024 10:26:44 PM
* Author: mikey
*/


#include "EEPROM_Writer.h"

#include <avr/io.h>

// default constructor
EEPROM_Writer::EEPROM_Writer()
{
	idx = 0;
} //EEPROM_Writer

void EEPROM_Writer::write_byte(uint8_t ch)
{
}
void EEPROM_Writer::delimiter() {}
void EEPROM_Writer::clear_eeprom() {}
