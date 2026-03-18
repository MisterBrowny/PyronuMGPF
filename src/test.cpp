#include "includes.h"

#define PRINT_TEST_OUTPUT	0
#define TEST_FCT_ANALOG		0

struTest Test;

void check_comutest (byte State)
{
	if (State == LOW)
	{// Si PUISSANCE ON => boucle while 
		while (COMU_PUISS_OUI == LOW)
		{
			ecran_blank();
			ecran_com_on();
			st7567s_refresh();

			SERIAL_DEBUG("Erreur - puissance active");

			if (Micro.Phase == MICRO_TEST)
			{
				Test.Step = TEST_WAIT;
				ecran_blank();
			}
		}
	}
	else if (State == HIGH)
	{// Si PUISSANCE OFF => boucle while 
		while (COMU_PUISS_NON == LOW)
		{
			ecran_blank();
			ecran_erreur_tir();
			st7567s_refresh();

			SERIAL_DEBUG("Erreur - puissance inactive tir impossible");
		}
	}

	// Cas d'erreur bouton
	if ((COMU_PUISS_NON == LOW) && (COMU_PUISS_OUI == LOW))
	{
		while(1)
		{
			ecran_blank();
			ecran_erreur_comu();
			st7567s_refresh();

			SERIAL_DEBUG("Erreur - comu puissance oui et non vus LOW");
		}
	}
	// else if ((COMU_PUISS_NON == HIGH) && (COMU_PUISS_OUI == HIGH))
	// {
	// 	while(1)
	// 	{
	// 		ecran_blank();
	// 		ecran_erreur_comu();
	// 		st7567s_refresh();

	// 		SERIAL_DEBUG("Erreur - comu puissance oui et non vus HIGH");
	// 	}
	// }
}

void check_NB_AT (byte State)
{
// TODO	
	if (State == LOW)
	{// Si NB_AT est à l'état bas, AT est inactif => boucle while (pas de tir possible)
		while (AT_SIGNAL == LOW)
		{
			ecran_erreur_tir();
			st7567s_refresh();

			if (Micro.Phase == MICRO_TEST)
			{
				Test.Step = TEST_WAIT;
				ecran_blank();
			}
		}
	}
	else if (State == HIGH)
	{// Si NB_AT est inactif => boucle while 
		while (COMU_PUISS_NON == LOW)
		{
			ecran_erreur_tir();
			st7567s_refresh();
		}
	}

	// Cas d'erreur bouton
	if (	((COMU_PUISS_NON == LOW) && (COMU_PUISS_OUI == LOW))
		||	((COMU_PUISS_NON == HIGH) && (COMU_PUISS_OUI == HIGH)))
	{
		while(1)
		{
			ecran_erreur_comu();
			st7567s_refresh();
		}
	}
}

byte check_program_0 (void)
{
	byte ret = false;
	
	if ((ID_TEST == 0) && (BP_ON == 0))	
	{
		SERIAL_DEBUG("Bouton Test and ON pressed, start program 0");

		ecran_blank();
		ecran_prog_0();
		st7567s_refresh();

		while ((ID_TEST == 0) || (BP_ON == 0));

		Micro.Mod.P_0 = true;
		
		ret = true;

		Micro.Step = MICRO_STEP_3;

		ecran_wait();
	}

	return ret;
}


void check_idtest (void)
{
	if ((ID_TEST == 0) && (BP_ON == 1))
	{
		SERIAL_DEBUG("Bouton test pressed, enter in programming mode");
		
		ecran_prog();
		st7567s_refresh();

		while(true)	{cf_rcv();}
	}
}

void check_bpon (void)
{
	Test.Time = millis();

	if (BP_ON == 0)
	{
		SERIAL_DEBUG("Bouton ON pressed");
		ecran_bp_on ();
		st7567s_refresh();
	
		while((BP_ON == 0) && (TempsInf(Test.Time, TDef1sec)));
	}

}

void check_UAlim (void)
{
	uint32_t 	temp = 0;
	char 		temp_tab[5] = {0};

	temp = analogReadMilliVolts(U_TEST_1A_ADC);

	//Test.U_Alim = (float) temp * CONVERSION_ADC;
	Test.U_Alim = temp * PONT_DIVISEUR;

	SERIAL_DEBUG(Test.U_Alim);

	//Test.U_Alim = Test.U_Alim * 100.0f;

	itoa((int) Test.U_Alim, &temp_tab[0], 10);

	if (Test.U_Alim < 10000.0f)
	{
		Ecran.Digit[0] = ' ';
		Ecran.Digit[1] = temp_tab[0];
		Ecran.Digit[2] = '.';
		Ecran.Digit[3] = temp_tab[1];
		Ecran.Digit[4] = temp_tab[2];
		Ecran.Digit[5] = temp_tab[3];
		Ecran.Digit[6] = 0;
	}
	else
	{
		Ecran.Digit[0] = temp_tab[0];
		Ecran.Digit[1] = temp_tab[1];
		Ecran.Digit[2] = '.';
		Ecran.Digit[3] = temp_tab[2];
		Ecran.Digit[4] = temp_tab[3];
		Ecran.Digit[5] = 0;
		Ecran.Digit[6] = 0;
	}
}

uint32_t check_UInfla (void)
{
	uint32_t temp = 0;

	digitalWrite(LOAD_TEST_20mA, HIGH);
	
	temp = analogReadMilliVolts(U_TEST_INF);

	digitalWrite(LOAD_TEST_20mA, LOW);

	//Test.U_Infla = (float) temp * CONVERSION_ADC;
	Test.U_Infla = temp;

	SERIAL_DEBUG(Test.U_Infla);
	
	//Test.U_Infla = Test.U_Infla * 100.0f;

	return (uint32_t) Test.U_Infla;
}

void test_update_analog (uint8_t number, uint8_t state)
{
// 	if (number > 8)
// 	{
// 		analog[0]
// 		char string[128];
//   sprintf(string, "%ld %ld %02X %02X %04X %04X %s", transactionCounter, errorCounter, unitId, functionCode, startingAddress, quantity, errorStrings[error]);
//   Serial.print(string);
// 	}
	#if DEBUG_PRINT
		char string[100];
		sprintf(string, "output number = %d, state = %02x", number, state);
		SERIAL_DEBUG(string);
	#endif

	modbus_analog_register[(((number-1) < 8) ? 0 : 1)] |= state << ((((number-1) < 8) ? (number-1) : (number-8-1)) * 2);

	#if DEBUG_PRINT
		sprintf(string, "modbus_analog_register[0]=0x%04X, modbus_analog_register[1]=0x%04X", modbus_analog_register[0], modbus_analog_register[1]);
		SERIAL_DEBUG(string);
	#endif
}

byte test_process (void)
{
	word temp;
	byte i, ret = false;
	
	switch(Test.Step)
	{
		case TEST_WAIT :
			if (Bouton[Bp_IdTest].State == 1)
			{
				Test.Step = TEST_ALIM;
			}
			break;

		case TEST_ALIM:
			check_comutest(LOW);

			Test.Cpt = 0;

			check_UAlim();

			Test.Step = TEST_WAIT_2;
			break;
		case TEST_WAIT_2 :
			if (Bouton[Bp_IdTest].State == 0)
			{
				if (Micro.Mod.P_0 == false)	{Test.Step = TEST_INFLA;}
				else						{Test.Step = TEST_INFLA_P0;}

				ecran_blank();
				check_comutest(LOW);
				Test.Time = millis();
				modbus_analog_register[0]=0;
				modbus_analog_register[1]=0;
				Test.no_display_refresh = true;
			}
			break;
		case TEST_INFLA:
			if (Cf.Data[Test.Cpt*3] == 0)
			{
				// Fin du test infla
				Test.Step = TEST_FIN_INFLA;
			}
			else if (TempsSup(Test.Time, TDef20ms))
			{
                if (Cf.Data[Test.Cpt*3] == PAUSE_VALUE) // MOD_V0010
                {
                    Test.Step = TEST_NO_INFLA_PRINT;
                }
                else
                {
                    register_one_tir_on(Cf.Data[Test.Cpt*3]);
                    Test.Step = TEST_INFLA_2;
                }
                Test.Time = millis();
            }
			break;
		case TEST_INFLA_2:	
			if (TempsSup(Test.Time, TDef20ms))
			{
				temp = check_UInfla();
				
				#if TEST_FCT_ANALOG
					if ((Cf.Data[Test.Cpt*3] % 4) == 0)
					{
						test_update_analog(Cf.Data[Test.Cpt*3], ANALOG_MOYEN);
						Test.Step = TEST_INFLA_OK;
					}
					else if ((Cf.Data[Test.Cpt*3] % 3) == 0)
					{
						test_update_analog(Cf.Data[Test.Cpt*3], ANALOG_OK);
						Test.Step = TEST_INFLA_NOK;
					}
					else if ((Cf.Data[Test.Cpt*3] % 2) == 0)
					{
						test_update_analog(Cf.Data[Test.Cpt*3], ANALOG_KO);
						Test.Step = TEST_INFLA_NOK;
					}
					else
					{
						Test.Step = TEST_INFLA_NOK;
					}	
				#else
					if (temp > DefValInflaNOK) 			
					{
						test_update_analog(Cf.Data[Test.Cpt*3], ANALOG_KO);
						Test.Step = TEST_INFLA_NOK;
					}
					else if (temp > DefValInflaMOYEN) 	
					{
						test_update_analog(Cf.Data[Test.Cpt*3], ANALOG_MOYEN);
						Test.Step = TEST_INFLA_NOK;
					}
					else
					{
						test_update_analog(Cf.Data[Test.Cpt*3], ANALOG_OK);
						Test.Step = TEST_INFLA_OK;
					}
				#endif

				// Affiche le num de l'infla testé
				#if PRINT_TEST_OUTPUT
					{
						char string [100];

						sprintf(string, "Out = %d\r\n Analog = %d", Cf.Data[Test.Cpt*3], temp);
						display.textflow(st7567sfGK::toptobottom);
						display.clear(st7567sfGK::colorblack);
						display.setFont(&FreeSans9pt7b);
						
						display.print(string);						
					}
				#endif
				register_raz();
				
				Test.Time = millis();
			}
			break;
        // debut MOD_V0010
        case TEST_NO_INFLA_PRINT:	
			/*if (TempsSup(Test.Time, TDef20ms))*/
			{
				Test.Step = TEST_NO_INFLA_PAUSE;

				// Affiche le num de la pause
				#if PRINT_TEST_OUTPUT
					ecran_print_num(Cf.Data[Test.Cpt*3]);
				#endif
				Test.Time = millis();
			}
			break;
        case TEST_NO_INFLA_PAUSE:
			/*if (TempsSup(Test.Time, TDef20ms))*/
			{
				if (++Test.Cpt > (NB_RELAY + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;} // MOD_V0010
				else                                            {Test.Step = TEST_INFLA;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
        // fin MOD_V0010
		case TEST_INFLA_OK:
			#if PRINT_TEST_OUTPUT
				if (TempsSup(Test.Time, TDef3sec))
			#endif
			{
				if (++Test.Cpt > (NB_RELAY + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;} // MOD_V0010
				else                                            {Test.Step = TEST_INFLA;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_NOK:
			#if PRINT_TEST_OUTPUT
				if (TempsSup(Test.Time, TDef3sec))
			#endif
			{
				if (++Test.Cpt > (NB_RELAY + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;}   // MOD_V0010
				else                                            {Test.Step = TEST_INFLA;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_FIN_INFLA:
			Test.no_display_refresh = false;
			if (Test.Cpt != 0)
			{
				ecran_print_num(Cf.Data[OFFSET_LAST_OUTPUT]);
			}
			else
			{
				Ecran.Digit[3] = ' ';
				Ecran.Digit[4] = ' ';
			}

			Ecran.Digit[0] = '-';
			Ecran.Digit[1] = '-';
			Ecran.Digit[2] = '-';

			Test.Step = TEST_FIN_INFLA_2;
			Test.Time = millis();
			break;
		case TEST_FIN_INFLA_2:
			if (TempsSup(Test.Time, TDef1sec))
			{
				Test.Step = TEST_PRINT_RESULT;

				Test.Time = millis();
			}
			break;
		case TEST_INFLA_P0:
			if (TempsSup(Test.Time, TDef20ms))
			{
				register_one_tir_on((byte) (Test.Cpt + 1));
				Test.Step = TEST_INFLA_2_P0;
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_2_P0:	
			if (TempsSup(Test.Time, TDef20ms))
			{
				temp = check_UInfla();

				if (temp > DefValInflaNOK) 			
				{
					test_update_analog((byte) (Test.Cpt + 1), ANALOG_KO);
					Test.Step = TEST_INFLA_NOK;
				}
				else if (temp > DefValInflaMOYEN) 	
				{
					test_update_analog((byte) (Test.Cpt + 1), ANALOG_MOYEN);
					Test.Step = TEST_INFLA_NOK;
				}
				else
				{
					test_update_analog((byte) (Test.Cpt + 1), ANALOG_OK);
					Test.Step = TEST_INFLA_OK;
				}

				// Affiche le num de l'infla testé
				#if PRINT_TEST_OUTPUT
					ecran_print_num((byte) (Test.Cpt + 1));
				#endif
				register_raz();
					
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_OK_P0:
			/*if (TempsSup(Test.Time, TDef20ms))*/
			{
				if (++Test.Cpt > (NB_RELAY - 1))	{Test.Step = TEST_FIN_INFLA_P0;}    // MOD_V0010
				else								{Test.Step = TEST_INFLA_P0;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_NOK_P0:
			/*if (TempsSup(Test.Time, TDef20ms))*/
			{
				if (++Test.Cpt > (NB_RELAY - 1))	{Test.Step = TEST_FIN_INFLA_P0;}    // MOD_V0010
				else                                {Test.Step = TEST_INFLA_P0;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_FIN_INFLA_P0:
			if (Test.Cpt != 0)
			{
				ecran_print_num(Test.Cpt);
			}
			else
			{
				Ecran.Digit[3] = ' ';
				Ecran.Digit[4] = ' ';
			}

			Ecran.Digit[0] = '-';
			Ecran.Digit[1] = '-';
			Ecran.Digit[2] = '-';
			
			Test.Step = TEST_FIN_INFLA_P0_2;
			Test.Time = millis();
			break;
		case TEST_FIN_INFLA_P0_2:
			if (TempsSup(Test.Time, TDef1sec))
			{
				Test.Step = TEST_PRINT_RESULT;

				Test.Time = millis();
			}
			break;
		case TEST_PRINT_RESULT:
			Test.print_result = true;
			Test.Step = TEST_WAIT_3;
			break;
		case TEST_WAIT_3:
			if (	(Bouton[Bp_IdTest].State == 0)
				/*||	(TempsSup(Test.Time, TDef10sec))*/)
			{
				Test.Step = TEST_WAIT_4;
				ecran_wait();
			}
			break;
		case TEST_WAIT_4:
			if (Bouton[Bp_IdTest].State == 1)
			{
				Micro.Phase = MICRO_WAIT;
			}
			break;
	}
	
	return ret;
}


