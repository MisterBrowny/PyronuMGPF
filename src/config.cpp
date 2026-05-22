#include "includes.h"

struConfig	Cf;

// on verifie que le checksum et bien le meme que la valeur en memoire OFFSET_CHECKSUM_1, OFFSET_CHECKSUM_2, OFFSET_CHECKSUM_3 et OFFSET_CHECKSUM_4 (4 octets)
static bool cf_checksum (void)
{
	uint32_t calcul;
	byte i, valid = false;
	
	for (i = 0, calcul = 0; i < (NB_TIR + NB_PAUSE_MAX); i ++)    // MOD_V0010
	{
		calcul += (uint32_t) ((uint32_t) (Cf.Data[i*CF_SECTOR_SIZE+1] << 16) + (uint32_t) (Cf.Data[i*CF_SECTOR_SIZE+2] << 8) + Cf.Data[i*CF_SECTOR_SIZE+3]);
	}

	if (	(((calcul >> 24) & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_1])   // MOD_V0010
		 &&	(((calcul >> 16) & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_2])   // MOD_V0010
		 &&	(((calcul >> 8) & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_3])    // MOD_V0010    
		 &&	((calcul & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_4]))          // MOD_V0010
	{
		valid = true;
		SERIAL_DEBUG("cf_checksum OK");
	}
	else
	{
		SERIAL_DEBUG("cf_checksum NOK");
	}
        
	if ((Cf.Data[OFFSET_CHECKSUM_1] == 0) && (Cf.Data[OFFSET_CHECKSUM_2] == 0) && (Cf.Data[OFFSET_CHECKSUM_3] == 0) && (Cf.Data[OFFSET_CHECKSUM_4] == 0))   // MOD_V0010
	{// Program_0
		Micro.Mod.P_00 = true;
	}
		
	return valid;
}

// on verifie que le dernier relais programme et bien le meme que la valeur en memoire OFFSET_LAST_OUTPUT
static bool cf_checkout (void)
{
	byte i, valid = false;
	byte last_out = 0;

	for (i = 0; i < (NB_TIR + NB_PAUSE_MAX); i ++)    // MOD_V0010
	{
		if (Cf.Data[i*CF_SECTOR_SIZE] == 0) 
		{
			if (i == 0)	{return valid;}						// erreur pas de sortie selectionnée

			if (last_out == 0)
			{
				last_out = Cf.Data[(i - 1) * CF_SECTOR_SIZE];	// save la valeur de la derniére sortie
			}
		}
		else
		{
			if (last_out != 0)	{return valid;}				// erreur une valeur a été mise à 0 avant
		}
	}

	if (	((last_out != 0) && (last_out == Cf.Data[OFFSET_LAST_OUTPUT]))
		||	((last_out == 0) && (Cf.Data[i*CF_SECTOR_SIZE] == Cf.Data[OFFSET_LAST_OUTPUT])))
	{
		valid = true;
		SERIAL_DEBUG("cf_checkout OK");
	}
	else
	{
		SERIAL_DEBUG("cf_checkout NOK");
	}
	
	return valid;
}

// on verifie qu'il n'y a pas d'appel de relais > à NB_TIR
// MOD_V0010 : on permet la valeur PAUSE_VALUE pour la pause
static bool cf_checkrange (void)
{
	byte i, valid = false;

	for (i = 0; i < (NB_TIR + NB_PAUSE_MAX); i ++)    // MOD_V0010
	{
		if (    (Cf.Data[i*CF_SECTOR_SIZE] > NB_TIR)
            &&  (Cf.Data[i*CF_SECTOR_SIZE] != PAUSE_VALUE)) // MOD_V0010
        {
			SERIAL_DEBUG("cf_checkrange NOK");
			SERIAL_DEBUG(i);
            return valid;
        }
	}

	SERIAL_DEBUG("cf_checkrange OK");
	valid = true;
	
	return valid;
}

byte cf_check (void)
{
	byte valid = false;

	SERIAL_DEBUG("Cf checks ...");
	
	if (cf_checksum() && cf_checkout() && cf_checkrange())
	{
		valid = true;
		SERIAL_DEBUG("Cf checks OK");
	}
	else
	{
		SERIAL_DEBUG("Cf checks NOK");
	}

	return valid;
}

// fonction de control de la validite du programme (sinon affichage "E")
void cf_check_and_display (void)
{
	byte configOk = false;
	word temp;

	eeprom_read_array(&Cf.Data[0], 0, CF_SIZE);

	configOk = cf_check();

	if (configOk == true)
	{
		char string_test[100];

		SERIAL_DEBUG("CONFIG OK");

		// affichage du checksum
		sprintf(string_test, "Seq=%02X%02X%02X%02X", Cf.Data[OFFSET_CHECKSUM_1], Cf.Data[OFFSET_CHECKSUM_2], Cf.Data[OFFSET_CHECKSUM_3], Cf.Data[OFFSET_CHECKSUM_4]);

		display.println(string_test);
		SERIAL_DEBUG(string_test);
		memcpy(Modbus.state.config, Cf.Data, CF_SIZE);
	}
	else
	{
		SERIAL_DEBUG("CONFIG NOT OK");	
		display.println("SEQ=Err Conf");
		while (true);
	}
}


void cf_rcv (void)
{
	byte i, temp = false;

	

	if (TempsSup(Cf.Time2,TDef100ms))
	{
		if (memcmp(Modbus.state.config, Cf.MemoData, CF_SIZE) != 0)
		{
			char string_test[100];

			SERIAL_DEBUG("Cf rcv from MODBUS");

			eeprom_read_array(&Cf.Data[0], 0, CF_SIZE);		// Read config

			sprintf(string_test, "EepSeq=%02X%02X%02X%02X", Cf.Data[OFFSET_CHECKSUM_1], Cf.Data[OFFSET_CHECKSUM_2], Cf.Data[OFFSET_CHECKSUM_3], Cf.Data[OFFSET_CHECKSUM_4]);
			SERIAL_DEBUG(string_test);
			
			memcpy(Cf.MemoData, Modbus.state.config, CF_SIZE);
			memcpy(Cf.Data, Modbus.state.config, CF_SIZE);

			sprintf(string_test, "RcvSeq=%02X%02X%02X%02X", Cf.Data[OFFSET_CHECKSUM_1], Cf.Data[OFFSET_CHECKSUM_2], Cf.Data[OFFSET_CHECKSUM_3], Cf.Data[OFFSET_CHECKSUM_4]);
			SERIAL_DEBUG(string_test);
				
			temp = cf_check();

			if (temp == false)
			{// config nok
				SERIAL_DEBUG("Cf rcv from MODBUS not ok");
				eeprom_read_array(&Cf.MemoData[0], 0, CF_SIZE);		// Read config
				memcpy(Modbus.state.config, Cf.MemoData, CF_SIZE);
				ecran_erreur_conf();
			}
			else
			{// config ok
				SERIAL_DEBUG("Cf OK from MODBUS, write to eeprom ...");

				eeprom_write_array(&Cf.Data[0], 0, CF_SIZE);	// Write config

				SERIAL_DEBUG("CF written to eeprom");

				for (i = 0; i < CF_SIZE; i ++)					// Raz config
				{
					Cf.Data[i] = 0;
				}

				eeprom_read_array(&Cf.Data[0], 0, CF_SIZE);		// Read config

				SERIAL_DEBUG("CF read from eeprom");
				
				temp = cf_check();								// 2éme check config

				if (temp == false)
				{// ecriture eeprom nok
					ecran_erreur_eepr();
					SERIAL_DEBUG("CF verified in eeprom KO");
				}
				else
				{// ecriture eeprom ok
					ecran_write_ok();
					memcpy(Modbus.state.config, Cf.Data, CF_SIZE);
					SERIAL_DEBUG("CF verified in eeprom OK");
				}
			}
			st7567s_refresh();
		}		
		
		Cf.Time2 = millis();

		if (TempsSup(Cf.Time1, TDef100ms)) {Cf.Index = 0;}// Rx time out
	}
	else if (Cf.Index == CF_SIZE)
	{// Rx complete
		SERIAL_DEBUG("Rx complete");

		temp = cf_check();								// Check config

		if (temp == false)
		{// config nok
			ecran_erreur_conf();
		}
		else
		{// config ok
			SERIAL_DEBUG("Cf OK, write to eeprom ...");

			eeprom_write_array(&Cf.Data[0], 0, CF_SIZE);	// Write config

			SERIAL_DEBUG("CF written to eeprom");

			for (i = 0; i < CF_SIZE; i ++)					// Raz config
			{
				Cf.Data[i] = 0;
			}

			eeprom_read_array(&Cf.Data[0], 0, CF_SIZE);		// Read config

			SERIAL_DEBUG("CF read from eeprom");
			
			temp = cf_check();								// 2éme check config

			if (temp == false)
			{// ecriture eeprom nok
				ecran_erreur_eepr();
				SERIAL_DEBUG("CF verified in eeprom KO");
			}
			else
			{// ecriture eeprom ok
				ecran_write_ok();
				memcpy(Modbus.state.config, Cf.Data, CF_SIZE);
				SERIAL_DEBUG("CF verified in eeprom OK");
			}
		}
		st7567s_refresh();

		Cf.Index = 0;									// Raz index
	}
	else
	{
		if (Serial.available()) 
		{
			Cf.Data[Cf.Index] = Serial.read();
			Serial.write(Cf.Data[Cf.Index]);
			Cf.Index ++;

			Cf.Time1 = millis();
		}
	}
}


