#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>

/* Macros to access bytes within words and words within longs */
#define MSB_BYTE(c)		((uint8_t) ((uint8_t) ((c) & 0xF0) >> 4))
#define LSB_BYTE(c)		((uint8_t) ((c) & 0x0F))
#define LOW_BYTE(w)     ((uint8_t) ((w) & 0xFF))
#define HIGH_BYTE(w)    ((uint8_t) (((w) >> 8) & 0xFF))
#define LOW_WORD(l)     ((uint16_t) ((l) & 0xFFFF))
#define HIGH_WORD(l)    ((uint16_t) (((l) >> 16) & 0xFFFF))

#define CHANGE_LOW_QUART(c1, c2)	((uint8_t) ((uint8_t) (c1 & 0xF0) | (uint8_t) (c2 & 0x0F)))
#define CHANGE_HIGH_QUART(c1, c2)	((uint8_t) ((uint8_t) (c1 & 0x0F) | (uint8_t) (c2 & 0xF0)))

void DecToStr (uint8_t value, char * result);
char HexToAscii(uint8_t Value);

#endif	/* UTILS_H */

