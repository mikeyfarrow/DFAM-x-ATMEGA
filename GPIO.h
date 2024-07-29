/*
 * GPIO.h
 *
 * Macros for pin/port names and uses.
 */ 


#ifndef _DFAM_GPIO_H_
#define _DFAM_GPIO_H_

#include <avr/io.h>
#include <avr/sfr_defs.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define USART_BAUD_RATE 31250 // MIDI Baud Rate
#define BAUD_RATE_BYTES (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~(_BV(bit)))
#define toggle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))


#define RX_COMPLETE_INTERRUPT         (1<<RXCIE0)
#define DATA_REGISTER_EMPTY_INTERRUPT (1<<UDRIE0)

/************************************************************************/
/*		LED outputs for debug											*/
/************************************************************************/
#define LED_BANK_PORT	PORTC
#define LED1			PORTC0
#define LED2			PORTC1
#define LED3			PORTC2

#define BILED1_PORT		PORTC
#define BILED1_R		PORTC4
#define BILED1_G		PORTC5

#define BILED2_PORT		PORTD
#define BILED2_R		PORTD2
#define BILED2_G		PORTD3



/************************************************************************/
/*     INPUTS: mode switch & sync button & learn btn                    */
/************************************************************************/
#define DDR_MODE_SW		DDRC
#define DD_MODE_SW		DDC3
#define MODE_SWITCH_PIN PINC
#define MODE_SWITCH		PINC3

#define DDR_SYNC_BTN	DDRB
#define DD_SYNC_BTN		DDB1
#define SYNC_BTN_PIN	PINB
#define SYNC_BTN		PINB1

#define DDR_LEARN_SW    DDRB
#define DD_LEARN_SW		DDB4
#define LEARN_SW_PIN	PINB
#define LEARN_SW		PINB4



/************************************************************************/
/*     SPI Output: MOSI, SCK, CS  -->  DAC                              */
/************************************************************************/
#define DAC_CS_PORT PORTB
#define DAC_CS		PB2
#define SPI_DDR		DDRB
//#define SPI_MISO	PORTB4
#define SPI_MOSI	PORTB3
#define SPI_SCK		PORTB5
#define SPI_SS		PORTB2
#define SPI_SPCR	SPCR
#define SPI_SPSR	SPSR
#define SPI_SPIF	SPIF
#define SPI_SPE		SPE
#define SPI_MSTR	MSTR



/************************************************************************/
/*		DIGITAL Outputs: Trigger (x2), V/oct (x2), Velocity (x2)		*/
/************************************************************************/
/* Port Registers */
#define ADV_PORT	PORTB
#define TRIG_PORT	PORTD
#define VEL_PORT	PORTD

/* Data Direction Registers */
#define DDR_ADV		DDRB
#define DDR_TRIG	DDRD
#define DDR_VEL		DDRD

/* Data Direction bits */
#define DD_ADV		DDB0
#define DD_TRIGA	DDD7
#define DD_TRIGB	DDD4
#define DD_VELA		DDD6
#define DD_VELB		DDD5

/* Output pins */
#define ADV_OUT		PORTB0
#define TRIG_A_OUT	PORTD7
#define TRIG_B_OUT	PORTD4
#define VEL_A_OUT	PORTD6
#define VEL_B_OUT	PORTD3

#define VEL_A_DUTY	OCR0A
#define VEL_B_DUTY	OCR0B

// // midi sync out
// #define DDR_SYNC_OUT	DDRC
// #define DD_SYNC_OUT		DDC0
// #define SYNC_OUT_PORT	PORTC
// #define SYNC_OUT		PORTC0


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
						
// Macros to enable and disable Output Compare A Match Interrupt
#define ENABLE_OCI1A()    (TIMSK1 |= (1 << OCIE1A))
#define DISABLE_OCI1A()   (TIMSK1 &= ~(1 << OCIE1A))

// Macros to enable and disable Output Compare B Match Interrupt
#define ENABLE_OCI1B()    (TIMSK1 |= (1 << OCIE1B))
#define DISABLE_OCI1B()   (TIMSK1 &= ~(1 << OCIE1B))

/* LED helpers */
void set_all_LEDs();
void clear_all_LEDs();
void bank(uint8_t i);
void bank_off(uint8_t i);


/* hardware initialization */
void init_led_outputs();
void init_digital_outputs();
void init_digital_inputs();
void init_midi_UART();
void init_DAC_SPI();
void init_pwm_output();
void init_milli_counter_timer();
void init_timer1();

#endif /* _DFAM_GPIO_H_ */