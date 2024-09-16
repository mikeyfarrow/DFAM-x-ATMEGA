/*
 * ROMLayout.h
 *
 * Created: 8/1/2024 4:18:20 PM
 *  Author: mikey
 */ 


#ifndef ROMLAYOUT_H_
#define ROMLAYOUT_H_

#include <avr/interrupt.h>

#include "MidiController.h"

size_t save_object_to_eeprom(Serializable* obj, uint8_t offset)
{
	size_t size = obj->size_bytes();
	uint8_t buf[size];
	obj->serialize((uint8_t*) buf);
	eeprom_write_block(buf, (void*) offset, size);
	return size;
}

void save_config(MidiController& mctl)
{
	cli(); // disable interrupts globally
	
	ledc_red();
	
	uint8_t offset = 0;
	
	eeprom_write_byte((uint8_t*) offset++, 0xBB);
	eeprom_write_byte((uint8_t*) offset++, 0xBB);
	
	offset += save_object_to_eeprom(&mctl.settings,			 offset);
	offset += save_object_to_eeprom(&mctl.cv_out_a.settings, offset);
	offset += save_object_to_eeprom(&mctl.cv_out_b.settings, offset);
	
	ledc_green();
	
	sei(); // enable interrupts globally
}

bool load_config(MidiController& mctl)
{
	ledc_red();
	
	if (eeprom_read_byte((uint8_t*) 0) != 0xBB || eeprom_read_byte((uint8_t*) 1) != 0xBB)
	{
		ledc_red();
		return false;
	}
	
	size_t size_mctl = mctl.settings.size_bytes();
	size_t size_cvA = mctl.cv_out_a.settings.size_bytes();
	size_t size_cvB = mctl.cv_out_b.settings.size_bytes();
	
	byte buf_mctl[size_mctl], buf_cvA[size_cvA], buf_cvB[size_cvB];
	
	size_t offset = 2; /* dummy 0xBB bytes */
	eeprom_read_block(buf_mctl, (void*) offset,							size_mctl);
	eeprom_read_block(buf_cvA,	(void*) size_mctl + offset,				size_cvA);
	eeprom_read_block(buf_cvB,	(void*) size_mctl + offset + size_cvA,	size_cvB);
	
	mctl.settings.deserialize(buf_mctl);
	mctl.cv_out_a.settings.deserialize(buf_cvA);
	mctl.cv_out_b.settings.deserialize(buf_cvB);
	
	ledc_green();
	
	return true;
}


#endif /* ROMLAYOUT_H_ */