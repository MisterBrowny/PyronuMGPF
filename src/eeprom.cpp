#include "includes.h"

byte eeprom_read (byte Address)
{
/*	EEADR = Address;
	
	EEPGD = 0;
	CFGS = 0;
	RD = 1;

	Nop();
	Nop();
	
	return EEDATA;*/
}


void eeprom_write (byte Data, byte Address)
{
/*	TODO 
EEADR = Address;
	EEDATA = Data;

	EEPGD = 0;
	CFGS = 0;
	WREN = 1;

	di();

	EECON2 = 0x55;
	EECON2 = 0xAA;

	WR = 1;
	while(WR);

	ei();

	WREN = 0;*/
}

void eeprom_read_array (byte *pData, byte Address, byte NbData)
{
	byte i;

	for (i = 0; i < NbData; i ++)
	{
		*pData = eeprom_read(Address);
		pData ++;
		Address ++;
	}
}

void eeprom_write_array (byte *pData, byte Address, byte NbData)
{
	byte i;

	for (i = 0; i < NbData; i ++)
	{
		eeprom_write(*pData, Address);
		pData ++;
		Address ++;
	}
}
