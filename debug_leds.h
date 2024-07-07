
#ifndef _DEBUG_LEDS_H_
#define _DEBUG_LEDS_H_ 1

#include <avr/io.h>
#include "utilities.h"

#define LED1 PORTC0
#define LED2 PORTC1
#define LED3 PORTC2
#define LED4 PORTC3
#define LED_BANK_PORT PORTC

#define BILED1_R PORTC4
#define BILED1_G PORTC5
#define BILED1_PORT PORTC

#define BILED2_R PORTD2
#define BILED2_G PORTD5
#define BILED2_PORT PORTD

uint8_t LED_BANK[4] = { LED1, LED2, LED3, LED4 };

#define set_bank()		LED_BANK_PORT |= (1<<LED1) | (1<<LED2) | (1<<LED3) | (1<<LED4)
#define clear_bank()	LED_BANK_PORT &= ~((1<<LED1) | (1<<LED2) | (1<<LED3) | (1<<LED4))
#define bank(i)			set_bit(LED_BANK_PORT, LED_BANK[i])
#define bank_off(i)		clear_bit(LED_BANK_PORT, LED_BANK[i]) 

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

void init_led_outputs()
{
	DDRC = _BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC3) | _BV(DDC4) | _BV(DDC5);
	DDRD |= _BV(DDD2) | _BV(DDD5);
	
	set_all_LEDs();
}

#endif /*_DEBUG_LEDS_H_ */