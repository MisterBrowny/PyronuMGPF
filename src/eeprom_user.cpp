#include "includes.h"

#include "EEPROM.h"

#define EEPROM_SIZE CF_SIZE

void eeprom_init (void)
{
	SERIAL_DEBUG("EEPROM init begins");
	
	if (!EEPROM.begin(CF_SIZE)) 
	{
		SERIAL_DEBUG("Failed to initialize EEPROM");
		SERIAL_DEBUG("Restarting...");
		delay(1000);
		ESP.restart();
	}
	EEPROM.end();
	
	SERIAL_DEBUG("EEPROM init ends");
}

void eeprom_read_array (uint8_t *pData, uint8_t Address, uint8_t NbData)
{
	uint8_t i;

	EEPROM.begin(EEPROM_SIZE);

	for (i = 0; i < NbData; i ++)
	{
		*pData = EEPROM.read(Address);
		pData ++;
		Address ++;
	}

	EEPROM.end();
}

void eeprom_write_array (uint8_t *pData, uint8_t Address, uint8_t NbData)
{
	uint8_t i;

	EEPROM.begin(EEPROM_SIZE);

	for (i = 0; i < NbData; i ++)
	{
		EEPROM.write(Address, *pData);
		pData ++;
		Address ++;
	}
	EEPROM.commit();

	EEPROM.end();
}

// Exemple d'utilisation de EEPROM ESP32
// //Constants
// #define EEPROM_SIZE 12

// void setup() {
//   //Init Serial USB
//   Serial.begin(115200);
//   Serial.println(F("Initialize System"));
//   //Init EEPROM
//   EEPROM.begin(EEPROM_SIZE);

//   //Write data into eeprom
//   int address = 0;
//   int boardId = 18;
//   EEPROM.write(address, boardId);//EEPROM.put(address, boardId);
//   address += sizeof(boardId); //update address value

//   float param = 26.5;
//   EEPROM.writeFloat(address, param);//EEPROM.put(address, param);
//   EEPROM.commit();

//   //Read data from eeprom
//   address = 0;
//   int readId;
//   readId = EEPROM.read(address); //EEPROM.get(address,readId);
//   Serial.print("Read Id = ");
//   Serial.println(readId);
//   address += sizeof(readId); //update address value

//   float readParam;
//   EEPROM.get(address, readParam); //readParam=EEPROM.readFloat(address);
//   Serial.print("Read param = ");
//   Serial.println(readParam);

//   EEPROM.end();
// }