/*
 * GPIO.cpp
 *
 * Created: 7/10/2024 4:34:01 PM
 *  Author: mikey
 */ 

#include "GPIO.h"
#include "utilities.h"

uint8_t LED_BANK[3] = { LED1, LED2, LED3 };		

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
