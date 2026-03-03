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
void register_write_one_high (uint8_t Output);
void register_one_led_on (uint8_t led);
void register_one_tir_on (uint8_t tir);
void register_raz (void);

#endif	/* REGISTER_H */

