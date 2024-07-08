/*
 * GPIO.h
 *
 * Macros for pin/port names and uses.
 */ 


#ifndef _DFAM_GPIO_H_
#define _DFAM_GPIO_H_



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
/*     INPUTS: Mode switch                                              */
/************************************************************************/
#define MODE_SWITCH_PIN PINC
#define MODE_SWITCH		PINC3




/************************************************************************/
/*     SPI Output: MOSI, SCK, CS  -->  DAC                              */
/************************************************************************/
#define DAC_CS_PORT PORTB
#define DAC_CS		PB2
#define SPI_DDR		DDRB
#define SPI_MISO	PORTB4
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





#endif /* _DFAM_GPIO_H_ */