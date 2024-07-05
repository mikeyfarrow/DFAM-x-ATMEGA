/*
 * MCP_4822.h 
 
 * handles all SPI communication between ATMEGA328P and the DAC, MCP4822
 *
 */ 


#ifndef MCP_4822_H_
#define MCP_4822_H_

#include <avr/io.h>

#define DAC_CS_PORT PORTB
#define DAC_CS PB2

#define DAC_ABSEL 7
#define DAC_IGN 6
#define DAC_GAIN 5
#define DAC_SHDN 4

// data is between 0-4095, config bits will be added
void write_DAC_byte(uint8_t channel, uint16_t data, uint8_t gain=1)
{
	DAC_CS_PORT &= ~(1<<DAC_CS); // set channel select pin low to enable DAC

	SPDR = (channel<<DAC_ABSEL) | (0<<DAC_IGN) | (gain<<DAC_GAIN) | (1<<DAC_SHDN) | ((data>>8) & 0x0F);
	while (!(SPSR & (1<<SPIF)))
	;
	SPDR = data & 0x00FF;
	while (!(SPSR & (1<<SPIF)))
	;
	DAC_CS_PORT |= (1<<DAC_CS);	// set channel select pin high to latch data
}

#endif /* MCP_4822_H_ */