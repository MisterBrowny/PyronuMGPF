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
			// st7567s_refresh()
			
			SERIAL_DEBUG("Erreur - puissance active");

			if (Micro.Phase == MICRO_TEST)
			{
				Test.Step = TEST_WAIT;
				register_raz();
				digitalWrite(LOAD_TEST_20mA, LOW);
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
			//st7567s_refresh();

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

		Micro.Step = MICRO_STEP_2;

		//ecran_wait();
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

		Micro.Phase = MICRO_PROG;

		eeprom_read_array(&Cf.MemoData[0], 0, CF_SIZE);
		memcpy((uint8_t *) &Modbus.state.config[0], Cf.MemoData, CF_SIZE);

		while(true)	
		{
			modbus_refresh();
			cf_rcv();
		}
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
	
		while(BP_ON == 0);
	}
}

void check_UAlim (void)
{
	uint32_t 	temp = 0;
	char 		string_test[25];

	temp = analogReadMilliVolts(U_TEST_1A_ADC);
	Test.U_Alim = temp * PONT_DIVISEUR;

	sprintf(string_test, "U ALIM =%d", Test.U_Alim);
	SERIAL_DEBUG(string_test);

	Modbus.state.alim = (uint16_t) Test.U_Alim;
}

uint32_t moy_analog (uint8_t pin, uint32_t nb_mesure)
{
	uint32_t temp = 0;
	uint32_t i = 0;

	for (i = 0; i < nb_mesure; i ++)
	{	
		temp += analogReadMilliVolts(pin);
	}

	temp = (uint32_t) ((float) temp / (float) nb_mesure);
	
	return temp;
}

uint32_t check_UInfla (void)
{
	uint32_t temp = 0;

	// Déplacer en début de test INFLA
	//digitalWrite(LOAD_TEST_20mA, HIGH);
	
	//temp = analogReadMilliVolts(U_TEST_INF_ADC);
	temp = moy_analog(U_TEST_INF_ADC, DefNbMesureINFLA);

	// Déplacer en début de test INFLA
	//digitalWrite(LOAD_TEST_20mA, LOW);

	Test.U_Infla = temp;

	SERIAL_DEBUG(Test.U_Infla);

	return (uint32_t) Test.U_Infla;
}

void test_update_analog (uint8_t number, uint8_t state)
{
	#if DEBUG_PRINT
		char string[100];
		sprintf(string, "output number = %d, state = %02x", number, state);
		SERIAL_DEBUG(string);
	#endif

	Modbus.state.analog[(((number-1) < 8) ? 0 : 1)] |= state << ((((number-1) < 8) ? (number-1) : (number-8-1)) * 2);

	#if DEBUG_PRINT
		sprintf(string, "Modbus.state.analog[0]=0x%04X, Modbus.state.analog[1]=0x%04X", Modbus.state.analog[0], Modbus.state.analog[1]);
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
				Test.Time = millis();
				
				Modbus.state.analog[0]=0;
				Modbus.state.analog[1]=0;
			}
			break;

		case TEST_ALIM:
			check_comutest(LOW);
			
			if (TempsSup(Test.Time, TDef20ms * (SLAVE_ID - 1)))
			{
				Test.Cpt = 0;

				check_UAlim();

				Test.Step = TEST_ALIM_1_A;
			}
			break;

		case TEST_ALIM_1_A:
			
				arm_UAlim_1A(false);

				Test.Step = TEST_WAIT_2;
			break;
			
		case TEST_WAIT_2 :
			//if (Bouton[Bp_IdTest].State == 0)
			{
				if (Micro.Mod.P_0 == false)	{Test.Step = TEST_INFLA;}
				else						{Test.Step = TEST_INFLA_P0;}

				ecran_blank();
				check_comutest(LOW);
				digitalWrite(LOAD_TEST_20mA, HIGH);
				Test.Time = millis();
			}
			break;
		case TEST_INFLA:
			if (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 0)
			{
				// Fin du test infla
				Test.Step = TEST_FIN_INFLA;
			}
			else if (TempsSup(Test.Time, TDef20ms))
			{
                if (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == PAUSE_VALUE) // MOD_V0010
                {
                    Test.Step = TEST_NO_INFLA_PRINT;
                }
                else
                {
                    register_one_tir_on(Cf.Data[Test.Cpt*CF_SECTOR_SIZE]);
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
					if (	(Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 1) || (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 4)
						||	(Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 9) || (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 12))
					{
						test_update_analog(Cf.Data[Test.Cpt*CF_SECTOR_SIZE], ANALOG_OK);
						Test.Step = TEST_INFLA_OK;
					}
					else if (	(Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 2) || (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 3)
							 ||	(Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 10) || (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 13))
					{
						test_update_analog(Cf.Data[Test.Cpt*CF_SECTOR_SIZE], ANALOG_MOYEN);
						Test.Step = TEST_INFLA_NOK;
					}
					else if (	(Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 5) || (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 8)
							 ||	(Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 14) || (Cf.Data[Test.Cpt*CF_SECTOR_SIZE] == 15))
					{
						test_update_analog(Cf.Data[Test.Cpt*CF_SECTOR_SIZE], ANALOG_KO);
						Test.Step = TEST_INFLA_NOK;
					}
					else
					{
						Test.Step = TEST_INFLA_NOK;
					}	
				#else
					if (temp > DefValInflaNOK) 			
					{
						test_update_analog(Cf.Data[Test.Cpt*CF_SECTOR_SIZE], ANALOG_KO);
						Test.Step = TEST_INFLA_NOK;
					}
					else if (temp > DefValInflaMOYEN) 	
					{
						test_update_analog(Cf.Data[Test.Cpt*CF_SECTOR_SIZE], ANALOG_MOYEN);
						Test.Step = TEST_INFLA_NOK;
					}
					else
					{
						test_update_analog(Cf.Data[Test.Cpt*CF_SECTOR_SIZE], ANALOG_OK);
						Test.Step = TEST_INFLA_OK;
					}
				#endif

				// Affiche le num de l'infla testé
				#if PRINT_TEST_OUTPUT
					{
						char string [100];

						sprintf(string, "Out = %d\r\n Analog = %d", Cf.Data[Test.Cpt*CF_SECTOR_SIZE], temp);
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
					ecran_print_num(Cf.Data[Test.Cpt*CF_SECTOR_SIZE]);
				#endif
				Test.Time = millis();
			}
			break;
        case TEST_NO_INFLA_PAUSE:
			/*if (TempsSup(Test.Time, TDef20ms))*/
			{
				if (++Test.Cpt > (NB_TIR + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;} // MOD_V0010
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
				if (++Test.Cpt > (NB_TIR + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;} // MOD_V0010
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
				if (++Test.Cpt > (NB_TIR + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;}   // MOD_V0010
				else                                            {Test.Step = TEST_INFLA;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_FIN_INFLA:
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
			digitalWrite(LOAD_TEST_20mA, LOW);
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
					Test.Step = TEST_INFLA_NOK_P0;
				}
				else if (temp > DefValInflaMOYEN) 	
				{
					test_update_analog((byte) (Test.Cpt + 1), ANALOG_MOYEN);
					Test.Step = TEST_INFLA_NOK_P0;
				}
				else
				{
					test_update_analog((byte) (Test.Cpt + 1), ANALOG_OK);
					Test.Step = TEST_INFLA_OK_P0;
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
				if (++Test.Cpt > (NB_TIR - 1))	{Test.Step = TEST_FIN_INFLA_P0;}    // MOD_V0010
				else								{Test.Step = TEST_INFLA_P0;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_NOK_P0:
			/*if (TempsSup(Test.Time, TDef20ms))*/
			{
				if (++Test.Cpt > (NB_TIR - 1))	{Test.Step = TEST_FIN_INFLA_P0;}    // MOD_V0010
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
			digitalWrite(LOAD_TEST_20mA, LOW);
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
				register_raz();
				Test.Step = TEST_WAIT_4;
				ecran_wait();
			}
			else
			{
				register_print_test_status();
			}
			break;
		case TEST_WAIT_4:
			if (Bouton[Bp_IdTest].State == 1)
			{
				Micro.Phase = MICRO_WAIT;
				Micro.Step = MICRO_STEP_3;
			}
			break;
	}
	
	return ret;
}


