
#ifndef _MKF_UTILITIES_H_
#define _MKF_UTILITIES_H_ 1

#include <avr/io.h>
#include <avr/sfr_defs.h>

#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~(_BV(bit)))

#endif /*_MKF_UTILITIES_H_ */