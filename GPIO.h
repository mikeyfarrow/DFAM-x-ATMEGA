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
#define LED_A_PORT	PORTC
#define LED_A_R		PORTC3
#define LED_A_G		PORTC2

#define LED_B_PORT	PORTD
#define LED_B_R		PORTD1
#define LED_B_G		PORTD2

#define LED_C_PORT	PORTC
#define LED_C_R		PORTC0
#define LED_C_G		PORTC1

/************************************************************************/
/*     INPUTS: mode switch & sync button & learn btn                    */
/************************************************************************/
#define DDR_MODE_SW		DDRD
#define DD_MODE_SW		DDD7
#define MODE_SWITCH_PIN PIND
#define MODE_SWITCH		PIND7

#define DDR_SYNC_BTN	DDRC
#define DD_SYNC_BTN		DDC4
#define SYNC_BTN_PIN	PINC
#define SYNC_BTN		PINC4

#define DDR_LEARN_SW    DDRC
#define DD_LEARN_SW		DDC5
#define LEARN_SW_PIN	PINC
#define LEARN_SW		PINC5



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
#define DD_TRIGA	DDD3
#define DD_TRIGB	DDD4
#define DD_VELA		DDD6
#define DD_VELB		DDD5

/* Output pins */
#define ADV_OUT		PORTB0
#define TRIG_A_OUT	PORTD3
#define TRIG_B_OUT	PORTD4
#define VEL_A_OUT	PORTD6
#define VEL_B_OUT	PORTD5

#define VEL_A_DUTY	OCR0A
#define VEL_B_DUTY	OCR0B

// Macros to enable and disable Output Compare A Match Interrupt
#define ENABLE_OCI1A()    (TIMSK1 |= (1 << OCIE1A))
#define DISABLE_OCI1A()   (TIMSK1 &= ~(1 << OCIE1A))

// Macros to enable and disable Output Compare B Match Interrupt
#define ENABLE_OCI1B()    (TIMSK1 |= (1 << OCIE1B))
#define DISABLE_OCI1B()   (TIMSK1 &= ~(1 << OCIE1B))

#define leda_green()	set_bit(LED_A_PORT, LED_A_G);\
						clear_bit(LED_A_PORT, LED_A_R);
#define leda_red()		set_bit(LED_A_PORT, LED_A_R);\
						clear_bit(LED_A_PORT, LED_A_G);
#define leda_off()		clear_bit(LED_A_PORT, LED_A_G);\
						clear_bit(LED_A_PORT, LED_A_R);

#define ledb_green()	set_bit(LED_B_PORT, LED_B_G);\
						clear_bit(LED_B_PORT, LED_B_R);
#define ledb_red()		set_bit(LED_B_PORT, LED_B_R);\
						clear_bit(LED_B_PORT, LED_B_G);
#define ledb_off()		clear_bit(LED_B_PORT, LED_B_G);\
						clear_bit(LED_B_PORT, LED_B_R);

#define ledc_green()	set_bit(LED_C_PORT, LED_C_G);\
						clear_bit(LED_C_PORT, LED_C_R);
#define ledc_red()		set_bit(LED_C_PORT, LED_C_R);\
						clear_bit(LED_C_PORT, LED_C_G);
#define ledc_off()		clear_bit(LED_C_PORT, LED_C_G);\
						clear_bit(LED_C_PORT, LED_C_R);

void hardware_init();

#endif /* _DFAM_GPIO_H_ */