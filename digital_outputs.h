/*
 * digital_outputs.h
 *
 * controls all gate, trigger, and PWM outputs (i.e. velocity...?)
 */ 


#ifndef DIGITAL_OUTPUTS_H_
#define DIGITAL_OUTPUTS_H_


/* Port Registers */
#define ADV_PORT PORTB
#define TRIG_PORT PORTD
#define VEL_PORT PORTD

/* Data Direction Registers */
#define DDR_ADV DDRB
#define DDR_TRIG DDRD
#define DDR_VEL DDRD

/* Data Direction bits */
#define DD_ADV DDB0
#define DD_TRIGA DDD7
#define DD_VELA DDD6
#define DD_TRIGB DDD4
#define DD_VELB DDD3

/* Pins for writing output */
#define ADV_OUT PORTB0
#define TRIG_A_OUT PORTD7
#define VEL_A_OUT PORTD6

#define TRIG_B_OUT PORTD4
#define VEL_B_OUT PORTD3

#define TRIG_DUR 2 // the number of times to run the interrupt for each trig

volatile uint8_t trig_A_timer = 0;
volatile uint8_t trig_B_timer = 0;

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
	Interrupt timer for Timer0, comp A, triggered every 1ms
	If the trigger has been high for TRIG_DUR, then trigger is cleared
*/
ISR(TIMER0_COMPA_vect) {
	trig_A_timer++;
	trig_B_timer++;
	if (trig_A_timer >= TRIG_DUR)
	{
		clear_bit(TRIG_PORT, TRIG_A_OUT);
	}
	if (trig_B_timer >= TRIG_DUR)
	{
		clear_bit(TRIG_PORT, TRIG_B_OUT);
	}
}

/*
	trigger_A - sends a pulse on the trigger A output 
*/
void trigger_A()
{
	set_bit(TRIG_PORT, TRIG_A_OUT);
	trig_A_timer = 0;
}
/*
	trigger_A - sends a pulse on the trigger A output lasting 
*/
void trigger_B()
{
	set_bit(TRIG_PORT, TRIG_B_OUT);
	trig_B_timer = 0;
}

/*
	advance_clock - sends a single pulse on the ADV/CLOCK output
*/
void advance_clock()
{
	set_bit(ADV_PORT, ADV_OUT);
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