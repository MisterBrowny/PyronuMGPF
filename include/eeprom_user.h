#ifndef EEPROM_USER_H
#define	EEPROM_USER_H

void eeprom_init (void);
void eeprom_read_array (uint8_t *pData, uint8_t Address, uint8_t NbData);
void eeprom_write_array (uint8_t *pData, uint8_t Address, uint8_t NbData);

#endif	/* EEPROM_H */

