
#ifndef _DEBUG_LEDS_H_
#define _DEBUG_LEDS_H_ 1

#include <avr/io.h>
#include "utilities.h"

#define LED_A1 PORTC0
#define LED_A2 PORTC1
#define LED_A3 PORTC2
#define LED_A4 PORTC3
#define BANK_A_PORT PORTC

#define BILED_R PORTC4
#define BILED_G PORTC5
#define BILED_PORT PORTC


#define LED_B1 PORTD2
#define LED_B2 PORTD3
#define LED_B3 PORTD4
#define LED_B4 PORTD5
#define BANK_B_PORT PORTD


uint8_t BANK_A_LEDS[4] = {LED_A1, LED_A2, LED_A3, LED_A4};
uint8_t BANK_B_LEDS[4] = {LED_B1, LED_B2, LED_B3, LED_B4};

#define bank_A_on()		BANK_A_PORT |= (1<<LED_A1) | (1<<LED_A2) | (1<<LED_A3) | (1<<LED_A4)
#define bank_A_off()	BANK_A_PORT &= ~((1<<LED_A1) | (1<<LED_A2) | (1<<LED_A3) | (1<<LED_A4))
#define bank_A(i)		set_bit(BANK_A_PORT, BANK_A_LEDS[i])


#define bank_B_on()		BANK_B_PORT |= (1<<LED_B1) | (1<<LED_B2) | (1<<LED_B3) | (1<<LED_B4)
#define bank_B_off()	BANK_B_PORT &= ~((1<<LED_B1) | (1<<LED_B2) | (1<<LED_B3) | (1<<LED_B4))
#define bank_B(i)		set_bit(BANK_B_PORT, BANK_B_LEDS[i])


void status_led_red()
{
	set_bit(PORTC, PORTC4);
	clear_bit(PORTC, PORTC5);
}

void status_led_green()
{
	clear_bit(PORTC, PORTC4);
	set_bit(PORTC, PORTC5);
}

void status_led_off()
{
	clear_bit(PORTC, PORTC4);
	clear_bit(PORTC, PORTC5);
}

void set_all_LEDs()
{
	status_led_green();
	PORTC |= (1<<LED_A1) | (1<<LED_A2) | (1<<LED_A3) | (1<<LED_A4);
	PORTD |= (1<<LED_B1) | (1<<LED_B2) | (1<<LED_B3) | (1<<LED_B4);
}

void clear_all_LEDs()
{
	status_led_off();
	PORTC &= ~((1<<LED_A1) | (1<<LED_A2) | (1<<LED_A3) | (1<<LED_A4));
	PORTD &= ~((1<<LED_B1) | (1<<LED_B2) | (1<<LED_B3) | (1<<LED_B4));
}

void init_led_outputs()
{
	// all C ports are outputs
	DDRC = 0xFF;
	
	// D2 - D5 are outputs
	DDRD |= (1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5);
	
	set_all_LEDs();
}

#endif /*_DEBUG_LEDS_H_ */