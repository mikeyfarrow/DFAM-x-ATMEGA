/*
 * MCP_4822.h 
 * 
 * handles all SPI communication between ATMEGA328P and the DAC, MCP4822,
 * including all V/octave outputs
 *
 */ 


#ifndef MCP_4822_H_
#define MCP_4822_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <avr/eeprom.h>

#define DAC_CS_PORT PORTB
#define DAC_CS PB2

#define MCP4822_ABSEL 7
#define MCP4822_IGN 6
#define MCP4822_GAIN 5
#define MCP4822_SHDN 4

// SPI Definitions
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

#define MIDI_NOTE_MIN 0
#define MIDI_NOTE_MAX 119
#define DAC_CAL_VALUE 34.133333d

// Loop until any current SPI transmissions have completed
#define spi_wait()	while (!(SPI_SPSR & (1 << SPI_SPIF)));

// Initialize the SPI as master
void init_DAC_SPI()
{
	// make the MOSI, SCK, and SS pins outputs
	SPI_DDR |= ( 1 << SPI_MOSI ) | ( 1 << SPI_SCK ) | ( 1 << SPI_SS );

	// make sure the MISO pin is input
	SPI_DDR &= ~( 1 << SPI_MISO );

	// set up the SPI module: SPI enabled, MSB first, master mode,
	//  clock polarity and phase = 0, F_osc/16
	SPI_SPCR = ( 1 << SPI_SPE ) | ( 1 << SPI_MSTR );// | ( 1 << SPI_SPR0 );
	SPI_SPSR = 1;     // set double SPI speed for F_osc/2
}

/*
	midi_to_data - converts a MIDI note into the data bits used by the DAC
		midi_note 0 -> C-1
*/
uint16_t midi_to_data(uint8_t midi_note)
{
	if (midi_note < MIDI_NOTE_MIN || midi_note > MIDI_NOTE_MAX)
	{
		// error?
		return 0;
	}
	else
	{
		return midi_note * DAC_CAL_VALUE; // add 0.5 ?
	}
}

// data is between 0-4095, config bits will be added 
void output_dac(uint8_t channel, uint16_t data)
{
	DAC_CS_PORT &= ~(1<<DAC_CS);	//pull CS low to enable DAC
	
	SPDR = (channel<<MCP4822_ABSEL) | (0<<MCP4822_IGN) | (0<<MCP4822_GAIN) | (1<<MCP4822_SHDN) | ((data>>8) & 0x0F);
	spi_wait();
	
	SPDR = data & 0x00FF;
	spi_wait();
	
	DAC_CS_PORT |= (1<<DAC_CS);		//pull CS high to latch data
}

#endif /* MCP_4822_H_ */