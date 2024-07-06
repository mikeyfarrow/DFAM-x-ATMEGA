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

volatile uint8_t TRIG_TIMER = 0;

void init_digital_outputs()
{
	DDR_ADV |= (1<<DD_ADV);
	DDR_TRIG |= (1<<DD_TRIG);
	DDR_VEL |= (1<<DD_VEL);
	
	// TODO: pull downs? ups?
}


/*
	Interrupt timer for Timer0, comp A, triggered every 1ms
	If the trigger has been high for 10 counts, then trigger is cleared
*/
ISR(TIMER0_COMPA_vect) {
	//PORTC ^= 0xFF;
	
	TRIG_TIMER++;
	if (TRIG_TIMER >= 25)
	{
		clear_bit(TRIG_PORT, TRIG_OUT);
	}
}


void init_trig_timer()
{
	cli(); // disable interrupts globally
	
	/**************************************/
	/*********   TIMER 0 config   *********/
	/**************************************/
	TCCR0A = 0; // clear timer registers
	TCCR0B = 0;
	TCNT0 = 0;

	OCR0A = 249; // 1000 Hz ==> (16,000,000  /((249+1) * 64))
	TCCR0A |= (1 << WGM01); // CTC
	TCCR0B |= (1 << CS01) | (1 << CS00); // Prescaler 64
	TIMSK0 |= (1 << OCIE0A); // Output Compare Match A Interrupt Enable
	
	sei(); // enable interrupts globally
}


void send_trigger()
{
	set_bit(TRIG_PORT, TRIG_OUT);
	TRIG_TIMER = 0;
}

void send_pulse()
{
	set_bit(ADV_PORT, ADV_OUT);
}

void send_burst_of_pulses()
{
	
}

#endif /* DIGITAL_OUTPUTS_H_ */