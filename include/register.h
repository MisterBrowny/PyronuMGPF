#ifndef REGISTER_H
#define	REGISTER_H

// SN74HC585

#define REGISTERS_NUMBER            4
#define ONE_REGISTER_OUPUT_NUMBER   8

#define REGISTER_OUTPUT_NUMBER      (REGISTERS_NUMBER * ONE_REGISTER_OUPUT_NUMBER)
#define REGISTER_LED_NUMBER         16
#define REGISTER_TIR_OUT_NUMBER     16

extern const uint8_t Led[REGISTER_LED_NUMBER];

extern const uint8_t Tir[REGISTER_TIR_OUT_NUMBER];


void register_init (void);
void register_write (uint8_t Output);

#endif	/* REGISTER_H */

