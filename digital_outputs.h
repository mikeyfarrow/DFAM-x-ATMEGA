/*
 * digital_outputs.h
 *
 * controls all gate, trigger, and PWM outputs (i.e. velocity...?)
 */ 


#ifndef DIGITAL_OUTPUTS_H_
#define DIGITAL_OUTPUTS_H_

#include "./GPIO.h"

volatile uint8_t trig_A_ticks = 0;
volatile uint8_t trig_B_ticks = 0;

void init_digital_outputs()
{
	DDR_ADV |= (1<<DD_ADV);
	
	DDR_TRIG |= (1<<DD_TRIGA);
	DDR_VEL |= (1<<DD_VELA);
	
	DDR_TRIG |= (1<<DD_TRIGB);
	DDR_VEL |= (1<<DD_VELB);
	
	// TODO: pull downs? ups?
}

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void trigger_A()
{
	set_bit(TRIG_PORT, TRIG_A_OUT);
	trig_A_ticks = 0;
}
/*
	trigger_A - sends a pulse on the trigger A output lasting 
*/
void trigger_B()
{
	set_bit(TRIG_PORT, TRIG_B_OUT);
	trig_B_ticks = 0;
}

/*
	advance_clock - sends a single pulse on the ADV/CLOCK output
*/
void advance_clock()
{
	set_bit(ADV_PORT, ADV_OUT);
	/* TODO: Do this instead with PWM? Timer interrupts? */
	for (int i = 0; i < 1000; i++)
	{
		/* do nothing, wait a while */
	}
	clear_bit(ADV_PORT, ADV_OUT);	
}


/*
	advance_clock - sends a number of pulses on the ADV/CLOCK output
*/
void advance_clock(uint8_t steps)
{
	for (int i = 0; i < steps; i++)
	{
		advance_clock();
	}
}

#endif /* DIGITAL_OUTPUTS_H_ */