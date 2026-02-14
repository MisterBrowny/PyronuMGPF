#ifndef EEPROM_H
#define	EEPROM_H

uint8_t eeprom_read (uint8_t Address);
void eeprom_write (uint8_t Data, uint8_t Address);


void eeprom_read_array (uint8_t *pData, uint8_t Address, uint8_t NbData);
void eeprom_write_array (uint8_t *pData, uint8_t Address, uint8_t NbData);

#endif	/* EEPROM_H */

