/*
* EEPROM_Writer.h
*
* Created: 7/5/2024 10:26:45 PM
* Author: mikey
*/


#ifndef __ROTARY_ENCODER_H__
#define __rotary_encoder_H__

#include <avr/io.h>
#include <avr/sfr_defs.h>

#include "./debug_leds.h"
#include "./digital_outputs.h"

#define ENC_PIN PINC
#define ENC_A PINC3
#define ENC_B PINC2

#define DDR_ENC DDRC
#define DD_ENC_A DDC3
#define DD_ENC_B DDC2

#define NUM_STATES 3

class Rotary_Encoder
{
	private:
	uint8_t last_A;
	uint8_t count;
	void update_LEDs()
	{
		switch (count)
		{
			case 0:
				bank_B_off();
				bank_B(0);
				//trigger();
				break;
			case 1:
				bank_B_off();
				bank_B(0);
				bank_B(1);
				break;
			case 2:
				bank_B_off();
				bank_B(0);
				bank_B(1);
				bank_B(2);
				break;
			default:
				break;
		}
	};

	public:
	Rotary_Encoder() 
	{
		// ensure the pins are inputs
		DDR_ENC &= ~(1<<DD_ENC_A);
		DDR_ENC &= ~(1<<DD_ENC_B);
		
		count = 0;
		last_A = bit_is_set(ENC_PIN, ENC_A);
		//last_B = bit_is_set(ENC_PIN, ENC_A);
	};
	
	void update()
	{
		uint8_t cur_A = bit_is_set(ENC_PIN, ENC_A);
		if (cur_A != last_A)
		{
			uint8_t cur_B = bit_is_set(ENC_PIN, ENC_B);
			if (cur_A != cur_B)
			{
				// CW rotation
				count++;
				status_led_green();
				PORTD ^= _BV(PORTD7);
			}
			else
			{
				// CCW rotation
				count--;
				status_led_red();
				PORTD ^= _BV(PORTD7);
			}
			count = count < 0 ? 2 : count;
			count = count > 2 ? 0 : count;
			last_A = cur_A;
			update_LEDs();
		}
		
	};
};

#endif //__ROTARY_ENCODER_H__
