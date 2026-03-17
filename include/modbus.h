#ifndef MODBUS_H
#define MODBUS_H

extern uint16_t modbus_analog_register[2];

// 0b00 "ABSENT", 0b01: "KO", 0b10: "MOYEN", 0b11: "OK" 
#define MASK_ANALOG_STATE 0x0003

#define ANALOG_ABSENT   0b00
#define ANALOG_KO       0b01
#define ANALOG_MOYEN    0b10
#define ANALOG_OK       0b11


void modbus_init (void);
void modbus_refresh (void);

#endif