#include "includes.h"

struConfig	Cf;

// on verifie que le checksum et bien le meme que la valeur en memoire OFFSET_CHECKSUM_1, OFFSET_CHECKSUM_2, OFFSET_CHECKSUM_3 et OFFSET_CHECKSUM_4 (4 octets)
static bool cf_checksum (void)
{
	dword calcul;
	byte i, valid = false;
	
	for (i = 0, calcul = 0; i < (NB_RELAY + NB_PAUSE_MAX); i ++)    // MOD_V0010
	{
		calcul += (dword) ((word) (Cf.Data[i*CF_SECTOR_SIZE+1] << 8) + Cf.Data[i*CF_SECTOR_SIZE+2]);
	}

	if (	(((calcul >> 24) & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_1])   // MOD_V0010
		 &&	(((calcul >> 16) & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_2])   // MOD_V0010
		 &&	(((calcul >> 8) & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_3])    // MOD_V0010    
		 &&	((calcul & 0x000000FF) == Cf.Data[OFFSET_CHECKSUM_4]))          // MOD_V0010
	{
		valid = true;
	}
        
	if ((Cf.Data[OFFSET_CHECKSUM_1] == 0) && (Cf.Data[OFFSET_CHECKSUM_2] == 0) && (Cf.Data[OFFSET_CHECKSUM_3] == 0) && (Cf.Data[OFFSET_CHECKSUM_4] == 0))   // MOD_V0010
	{// Program_0
		Micro.Mod.P_00 = true;
	}

	if (calcul > 0x0000FFFF)	{Cf.IsLong = true;}
		
	return valid;
}

// on verifie que le dernier relais programme et bien le meme que la valeur en memoire OFFSET_LAST_OUTPUT
static bool cf_checkout (void)
{
	byte i, valid = false;
	byte last_out = 0;

	for (i = 0; i < (NB_RELAY + NB_PAUSE_MAX); i ++)    // MOD_V0010
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
	}
	
	return valid;
}

// on verifie qu'il n'y a pas d'appel de relais > à NB_RELAY
// MOD_V0010 : on permet la valeur PAUSE_VALUE pour la pause
static bool cf_checkrange (void)
{
	byte i, valid = false;

	for (i = 0; i < (NB_RELAY + NB_PAUSE_MAX); i ++)    // MOD_V0010
	{
		if (    (Cf.Data[i*CF_SECTOR_SIZE] > NB_RELAY)
            &&  (Cf.Data[i*CF_SECTOR_SIZE] != PAUSE_VALUE)) // MOD_V0010
        {
            return valid;
        }
	}

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
		SERIAL_DEBUG("CONFIG OK");

		// affichage du checksum
		Ecran.Digit[0] = HexToAscii(MSB_BYTE(Cf.Data[OFFSET_CHECKSUM_2]));
		Ecran.Digit[1] = HexToAscii(LSB_BYTE(Cf.Data[OFFSET_CHECKSUM_2]));
		Ecran.Digit[2] = HexToAscii(MSB_BYTE(Cf.Data[OFFSET_CHECKSUM_3]));
		Ecran.Digit[3] = HexToAscii(LSB_BYTE(Cf.Data[OFFSET_CHECKSUM_3]));
		Ecran.Digit[4] = HexToAscii(MSB_BYTE(Cf.Data[OFFSET_CHECKSUM_4]));
		Ecran.Digit[5] = HexToAscii(LSB_BYTE(Cf.Data[OFFSET_CHECKSUM_4]));
		Ecran.Digit[6] = 0;
		st7567s_refresh();
	}
	else
	{
		SERIAL_DEBUG("CONFIG NOT OK");	
		ecran_erreur_conf();
		st7567s_refresh();
		while (true);
	}
}

// TODO CF_RCV
void cf_rcv (void)
{
	byte i, temp = false;
	
	if (TempsSup(Cf.Time2,TDef100ms))
	{
		//SERIAL_DEBUG("Cf receives timeout");

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

			SERIAL_DEBUG("CF written to eeprom");

			eeprom_read_array(&Cf.Data[0], 0, CF_SIZE);		// Read config

			temp = cf_check();								// 2éme check config

			if (temp == false)
			{// ecriture eeprom nok
				ecran_erreur_eepr();
				SERIAL_DEBUG("CF verified in eeprom KO");
			}
			else
			{// ecriture eeprom ok
				ecran_write_ok();
				SERIAL_DEBUG("CF verified in eeprom OK");
			}
		}
		st7567s_refresh();

		Cf.Index = 0;									// Raz index
	}
	else
	{
		// Boucle de code faite dans 
		// 		Cf.Data[Cf.Index] = RCREG;
		// 		TXREG = Cf.Data[Cf.Index];

		// 		Cf.Index ++;

		// 		Cf.Time1 = Cptms;

		if (Serial.available()) 
		{
			Cf.Data[Cf.Index] = Serial.read();
			Serial.write(Cf.Data[Cf.Index]);
			Cf.Index ++;

			Cf.Time1 = millis();
		}
	}
}


