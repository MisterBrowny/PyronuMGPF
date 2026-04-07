#ifndef MODBUS_H
#define MODBUS_H

#include "config.h"

// 0b00 "ABSENT", 0b01: "KO", 0b10: "MOYEN", 0b11: "OK" 
#define MASK_ANALOG_STATE 0x0003

#define ANALOG_ABSENT   0b00
#define ANALOG_KO       0b01
#define ANALOG_MOYEN    0b10
#define ANALOG_OK       0b11

typedef struct StructStatut {
	uint16_t global_state;               // registre 0    
	uint16_t analog[2];                  // registre 1 + 2 (32 bits) : pour les 16 entrées valeur 0b00 "ABSENT", 0b01: "KO", 0b10: "MOYEN", 0b11: "OK" 
	uint16_t alim;                       // registre 3 : tension en mV
	uint16_t alim_1A;                    // registre 4 : tension à 1A en mV
	uint16_t config[CF_SIZE/2];
}struStatut;

#define DEF_HOLD_REG    (sizeof(struStatut) / 2)

typedef struct StructModbus {
	union {
        struStatut  state;
        uint16_t    holdingRegisters[DEF_HOLD_REG];
    };
}struModbus;

extern struModbus Modbus;

void modbus_init (void);
void modbus_refresh (void);

#endif