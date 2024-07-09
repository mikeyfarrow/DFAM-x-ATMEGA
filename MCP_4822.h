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

#include "GPIO.h"

#define MCP4822_ABSEL 7
#define MCP4822_IGN 6
#define MCP4822_GAIN 5
#define MCP4822_SHDN 4


/*
MIDI notes are shifted down by MIDI_NOTE_MIN so that the lowest note is 0.
	
For a range of 99 notes:
	- DAC is 12-bit so highest possible value is 4095.
	- Scale the notes so that midi note 0 --> 0x000
							    and note 87 --> 0xFFF (4095)
	- i.e. midi notes increase in steps of 4095 / 87 = 47.069 mV per step
	- therefore:
		- one octave => 12 steps/oct * 47.069 mV/step = 564.828 mV/oct
		- gain		 => 1000 mV/oct / 564.828 mV/oct  = 1.77x gain 
*/

#define MIDI_NOTE_MIN 24
#define MIDI_NOTE_MAX 111
#define DAC_CAL_VALUE 47.068966d

// For a 120 note range:
//#define MIDI_NOTE_MIN 0
//#define MIDI_NOTE_MAX 119
//#define DAC_CAL_VALUE 34.411765d // --> gain = 2.42


// Loop until any current SPI transmissions have completed
#define spi_wait()	while (!(SPI_SPSR & (1 << SPI_SPIF)));

// Initialize the SPI as master
void init_DAC_SPI()
{
	// make the MOSI, SCK, and SS pins outputs
	SPI_DDR |= ( 1 << SPI_MOSI ) | ( 1 << SPI_SCK ) | ( 1 << SPI_SS );

	// TODO: no it is not, not used by DAC
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
		return (midi_note - MIDI_NOTE_MIN) * DAC_CAL_VALUE; // add 0.5 ?
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