
#ifndef _DEBUG_LEDS_H_
#define _DEBUG_LEDS_H_ 1



/*
#include <avr/io.h>
#include "utilities.h"
#include "GPIO.h"
uint8_t LED_BANK[3] = { LED1, LED2, LED3 };

#define set_bank()		LED_BANK_PORT |= (1<<LED1) | (1<<LED2) | (1<<LED3)
#define clear_bank()	LED_BANK_PORT &= ~((1<<LED1) | (1<<LED2) | (1<<LED3))

#define status1_red()   set_bit(BILED1_PORT, BILED1_R); \
						clear_bit(BILED1_PORT, BILED1_G);	
#define status1_green()   set_bit(BILED1_PORT, BILED1_G); \
						  clear_bit(BILED1_PORT, BILED1_R);
#define status1_off()   clear_bit(BILED1_PORT, BILED1_G); \
						clear_bit(BILED1_PORT, BILED1_R);
						
#define status2_red()   set_bit(BILED2_PORT, BILED2_R); \
						clear_bit(BILED2_PORT, BILED2_G);	
#define status2_green()   set_bit(BILED2_PORT, BILED2_G); \
						  clear_bit(BILED2_PORT, BILED2_R);
#define status2_off()   clear_bit(BILED2_PORT, BILED2_G); \
						clear_bit(BILED2_PORT, BILED2_R);
						
void set_all_LEDs()
{
	status1_green();
	status2_green();
	set_bank();
}

void clear_all_LEDs()
{
	status1_off();
	status2_off();
	clear_bank();
}

void bank(uint8_t i) {
	set_bit(LED_BANK_PORT, LED_BANK[i]);
}

void bank_off(uint8_t i) {
	clear_bit(LED_BANK_PORT, LED_BANK[i]);
}
*/

#endif /*_DEBUG_LEDS_H_ */