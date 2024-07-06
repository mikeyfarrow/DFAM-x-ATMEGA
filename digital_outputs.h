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
#define DD_TRIG DDD7
#define DD_VEL DDD6

/* Pins for writing output */
#define ADV_OUT PORTB0
#define TRIG_OUT PORTD7
#define VEL_OUT PORTD6

void init_digital_outputs()
{
	DDR_ADV |= (1<<DD_ADV);
	DDR_TRIG |= (1<<DD_TRIG);
	DDR_VEL |= (1<<DD_VEL);
	
	// TODO: pull downs? ups?
}

void send_trigger()
{
	set_bit(TRIG_PORT, TRIG_OUT);
}

void send_pulse()
{
	set_bit(ADV_PORT, ADV_OUT);
}

void send_burst_of_pulses()
{
	
}

#endif /* DIGITAL_OUTPUTS_H_ */