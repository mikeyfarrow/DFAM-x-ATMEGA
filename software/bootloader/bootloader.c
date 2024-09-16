// Copyright 2011 Emilie Gillet.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Bootloader supporting MIDI SysEx update.
//
// Caveat: assumes the firmware flashing is always done from first to last
// block, in increasing order. Random access flashing is not supported!

// CPU main clock frequency in Hz
#define F_CPU 16000000UL

#include <inttypes.h>
#include <avr/boot.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>

#define DDR_SYNC_BTN DDRC
#define DD_SYNC_BTN DDC4
#define SYNC_BTN_PIN PINC
#define SYNC_BTN PINC4

#define DDR_LEARN_SW    DDRC
#define DD_LEARN_SW		DDC5
#define LEARN_SW_PIN	PINC
#define LEARN_SW		PINC5

#define LED_A_PORT PORTC
#define LED_A_R PORTC3
#define LED_A_G PORTC2

#define LED_B_PORT PORTD
#define LED_B_R PORTD1
#define LED_B_G PORTD2

#define LED_C_PORT PORTC
#define LED_C_R PORTC0
#define LED_C_G PORTC1

#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~(_BV(bit)))
#define toggle_bit(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))

#define leda_green()            \
  set_bit(LED_A_PORT, LED_A_G); \
  clear_bit(LED_A_PORT, LED_A_R);
#define leda_red()              \
  set_bit(LED_A_PORT, LED_A_R); \
  clear_bit(LED_A_PORT, LED_A_G);
#define leda_off()                \
  clear_bit(LED_A_PORT, LED_A_G); \
  clear_bit(LED_A_PORT, LED_A_R);

#define ledb_green()            \
  set_bit(LED_B_PORT, LED_B_G); \
  clear_bit(LED_B_PORT, LED_B_R);
#define ledb_red()              \
  set_bit(LED_B_PORT, LED_B_R); \
  clear_bit(LED_B_PORT, LED_B_G);
#define ledb_off()                \
  clear_bit(LED_B_PORT, LED_B_G); \
  clear_bit(LED_B_PORT, LED_B_R);

#define ledc_green()            \
  set_bit(LED_C_PORT, LED_C_G); \
  clear_bit(LED_C_PORT, LED_C_R);
#define ledc_red()              \
  set_bit(LED_C_PORT, LED_C_R); \
  clear_bit(LED_C_PORT, LED_C_G);
#define ledc_off()                \
  clear_bit(LED_C_PORT, LED_C_G); \
  clear_bit(LED_C_PORT, LED_C_R);

#define all_off() \
  leda_off();     \
  ledb_off();     \
  ledc_off();
#define all_green() \
  leda_green();     \
  ledb_green();     \
  ledc_green();
#define all_red() \
  leda_red();     \
  ledb_red();     \
  ledc_red();

uint16_t page = 0;
uint8_t rx_buffer[SPM_PAGESIZE + 1];

static const uint8_t sysex_header[] = {
    0xf0,             // <SysEx>
    0x00, 0x21, 0x02, // Manufacturer ID for Mutable instruments.
    0x00, 0x7f,       // Product ID for "any other project".
};

enum SysExReceptionState
{
  MATCHING_HEADER = 0,
  MATCHING_OLD_HEADER = 1,
  READING_COMMAND = 2,
  READING_DATA = 3,
};

static const uint16_t kUartPrescaler = (F_CPU / (16L * 31250)) - 1;

void init();
uint8_t bootloader_active();
void midi_rx_loop();
void write_status_leds(uint8_t pattern);

inline void init()
{
  cli();

  /********** LED output  *************/
  DDRC |= _BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC3);
  DDRD |= _BV(DDD1) | _BV(DDD2);

  /********** Push buttons  *************/
  DDR_SYNC_BTN &= ~_BV(DD_SYNC_BTN);
  DDR_LEARN_SW &= ~_BV(DD_LEARN_SW);

  /********** MIDI Rx UART  *************/
  UCSR0A &= ~_BV(U2X0);
  UBRR0H = kUartPrescaler >> 8;
  UBRR0L = kUartPrescaler;
  UCSR0B |= _BV(RXEN0);
}

inline void write_status_leds(uint8_t pattern)
{
  /* TODO: indicate progress ? */
}

inline uint8_t bootloader_active()
{
  /* If both push buttons ("Sync" and "Learn") are held, activate the bootloader */
  return !bit_is_set(SYNC_BTN_PIN, SYNC_BTN) && !bit_is_set(LEARN_SW_PIN, LEARN_SW);
}

int write_page(const uint32_t addr_cur_page, const uint8_t *program_buffer, const uint32_t program_buffer_size)
{
  toggle_bit(LED_C_PORT, LED_C_G);
  boot_page_erase(addr_cur_page);
  boot_spm_busy_wait(); // Wait until the memory is erased.

  int count = 0;

  // iterate through the page, one word (two bytes) at a time
  for (uint16_t i = 0; i < SPM_PAGESIZE; i += 2)
  {
    uint16_t current_word = 0;
    // Set up a little-endian word and point to the next word
    current_word = *program_buffer++;
    current_word |= (*program_buffer++) << 8;
    count += 2;

    boot_page_fill(addr_cur_page + i, current_word);
  }

  boot_page_write(addr_cur_page); // Store buffer in a page of flash memory.
  boot_spm_busy_wait();                  // Wait until the page is written.
  return count;
}

inline void midi_rx_loop()
{
  uint8_t byte;
  uint16_t bytes_read = 0;
  uint16_t rx_buffer_index;
  uint8_t state = MATCHING_HEADER;
  uint8_t checksum;
  uint8_t sysex_commands[2];
  uint8_t current_led = 1;
  uint8_t status = 0;
  uint8_t progress_counter = 0;

  page = 0;
  write_status_leds(0x55);
  while (1)
  {
    while (!(UCSR0A & _BV(RXC0)))
      ;
    byte = UDR0;
    // In case we see a realtime message in the stream, safely ignore it.
    if (byte > 0xf0 && byte != 0xf7)
    {
      continue;
    }
    write_status_leds(status);
    switch (state)
    {
    case MATCHING_HEADER:
      if (byte == sysex_header[bytes_read])
      {
        ++bytes_read;
        if (bytes_read == sizeof(sysex_header))
        {
          bytes_read = 0;
          state = READING_COMMAND;
        }
      }
      else
      {
        bytes_read = 0;
      }
      break;

    case READING_COMMAND:
      if (byte < 0x80)
      {
        sysex_commands[bytes_read++] = byte;
        if (bytes_read == 2)
        {
          bytes_read = 0;
          rx_buffer_index = 0;
          checksum = 0;
          state = READING_DATA;
        }
      }
      else
      {
        state = MATCHING_HEADER;
        current_led = 1;
        status = 0;
        bytes_read = 0;
      }
      break;

    case READING_DATA:
      if (byte < 0x80)
      {
        if (bytes_read & 1)
        {
          rx_buffer[rx_buffer_index] |= byte & 0xf;
          if (rx_buffer_index < SPM_PAGESIZE)
          {
            checksum += rx_buffer[rx_buffer_index];
          }
          ++rx_buffer_index;
        }
        else
        {
          rx_buffer[rx_buffer_index] = (byte << 4);
        }
        ++bytes_read;
      }
      else if (byte == 0xf7)
      {
        if (sysex_commands[0] == 0x7f &&
            sysex_commands[1] == 0x00 &&
            bytes_read == 0)
        {
          // Reset.
          return;
        }
        else if (rx_buffer_index == SPM_PAGESIZE + 1 &&
                 sysex_commands[0] == 0x7e &&
                 sysex_commands[1] == 0x00 &&
                 rx_buffer[rx_buffer_index - 1] == checksum)
        {
          // Block write.
          write_page(page, rx_buffer, page + SPM_PAGESIZE);
          page += SPM_PAGESIZE;
          ++progress_counter;
          if (progress_counter == 32)
          {
            status |= current_led;
            current_led <<= 1;
            if (current_led == 0)
            {
              current_led = 1;
              status = 0;
            }
            progress_counter = 0;
          }
          status ^= current_led;
        }
        else
        {
          current_led = 1;
          status = 0;
        }
        state = MATCHING_HEADER;
        bytes_read = 0;
      }
      break;
    }
  }
}

int main(void)
{
  /* MI's Watchdog Timer logic... what is this for? */
  // uint8_t watchdog_status = MCUSR;
  // MCUSR &= ~(1 << WDRF); // MCUSR = 0;
  // WDTCSR |= _BV(WDCE) | _BV(WDE);
  // WDTCSR = 0;

  init();
  
  if (pgm_read_word(0) == 0xFFFF || bootloader_active())
  {
    // Blink before entering MIDI loop
    for (uint8_t i = 0; i < 3; i++)
    {
      all_red();
      _delay_ms(900);
      all_off();
      _delay_ms(100);
    }
    
    ledc_green();
    midi_rx_loop();
    all_green();

    // Re-enable RWW-section. We need this to be able to jump back
    // to the application after bootloading.
    boot_rww_enable();

    _delay_ms(2000);
    all_off();
  }

  asm("jmp 0");
  // main_entry_point();
}
