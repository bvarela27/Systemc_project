#ifndef BIT26
#define BIT26
#include <stdint.h>

#define SCALE 22
#define LENGTH 26
#define FILLER 32-26

#define DoubleToFixed(x) ((uint32_t)(x*(double)(1<<SCALE)))&0x03FFFFFF
#define FixedToDouble(x) ((double)(x|(((x>>(LENGTH-1))*63)<<LENGTH))/(double)(1<<SCALE))
#endif
