#include "includes.h"

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
	int 	temp = 0;
	char 	temp_tab[5] = {0};

	temp = analogRead(U_TEST_1A_ADC);

	Test.U_Alim = (float) temp * CONVERSION_ADC;
	Test.U_Alim = Test.U_Alim * PONT_DIVISEUR;

	SERIAL_DEBUG(Test.U_Alim);

	Test.U_Alim = Test.U_Alim * 100.0f;

	itoa((int) Test.U_Alim, &temp_tab[0], 10);

	if (Test.U_Alim < 1000.0f)
	{
		Ecran.Digit[0] = ' ';
		Ecran.Digit[1] = temp_tab[0];
		Ecran.Digit[2] = '.';
		Ecran.Digit[3] = temp_tab[1];
		Ecran.Digit[4] = temp_tab[2];
		Ecran.Digit[5] = 0;
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

int check_UInfla (void)
{
	int temp = 0;

	digitalWrite(LOAD_TEST_20mA, HIGH);
	
	temp = analogRead(U_TEST_INF);

	digitalWrite(LOAD_TEST_20mA, LOW);

	Test.U_Infla = (float) temp * CONVERSION_ADC;

	SERIAL_DEBUG(Test.U_Infla);
	
	Test.U_Infla = Test.U_Infla * 100.0f;

	return (int) Test.U_Infla;
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
                    register_write(Cf.Data[Test.Cpt*3]);
                    Test.Step = TEST_INFLA_2;
                }
                Test.Time = millis();
            }
			break;
		case TEST_INFLA_2:	
			if (TempsSup(Test.Time, TDef20ms))
			{
				temp = check_UInfla();
				// Si tension > 0.94V, infla pas OK
				if (temp > DefValInflaOK) 	{Test.Step = TEST_INFLA_NOK;}
				else						{Test.Step = TEST_INFLA_OK;}

				// Affiche le num de l'infla testé
				ecran_print_num(Cf.Data[Test.Cpt*3]);
				register_write(0);
				
				Test.Time = millis();
			}
			break;
        // debut MOD_V0010
        case TEST_NO_INFLA_PRINT:	
			if (TempsSup(Test.Time, TDef20ms))
			{
				Test.Step = TEST_NO_INFLA_PAUSE;

				// Affiche le num de la pause
				ecran_print_num(Cf.Data[Test.Cpt*3]);
				Test.Time = millis();
			}
			break;
        case TEST_NO_INFLA_PAUSE:
			if (TempsSup(Test.Time, TDef100ms))
			{
				if (++Test.Cpt > (NB_RELAY + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;} // MOD_V0010
				else                                            {Test.Step = TEST_INFLA;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
        // fin MOD_V0010
		case TEST_INFLA_OK:
			if (TempsSup(Test.Time, TDef20ms))
			{
				if (++Test.Cpt > (NB_RELAY + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;} // MOD_V0010
				else                                            {Test.Step = TEST_INFLA;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_NOK:
			if (	(Bouton[Bp_IdTest].State == 0)
				&&	(TempsSup(Test.Time, TDef500ms)))
			{
				if (++Test.Cpt > (NB_RELAY + NB_PAUSE_MAX - 1))	{Test.Step = TEST_FIN_INFLA;}   // MOD_V0010
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
			break;
		case TEST_FIN_INFLA_2:
			if (Bouton[Bp_IdTest].State == 0)
			{
				Test.Step = TEST_WAIT_3;

				Test.Time = millis();
			}
			break;
		case TEST_INFLA_P0:
			if (TempsSup(Test.Time, TDef20ms))
			{
				register_write((byte) (Test.Cpt + 1));
				Test.Step = TEST_INFLA_2_P0;
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_2_P0:	
			if (TempsSup(Test.Time, TDef20ms))
			{
				temp = check_UInfla();

				// Si tension > 0.94V, infla pas OK
				if (temp > DefValInflaOK) 	{Test.Step = TEST_INFLA_NOK_P0;}
				else						{Test.Step = TEST_INFLA_OK_P0;}

				// Affiche le num de l'infla testé
				ecran_print_num((byte) (Test.Cpt + 1));
				register_write(0);
					
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_OK_P0:
			if (TempsSup(Test.Time, TDef20ms))
			{
				if (++Test.Cpt > (NB_RELAY - 1))	{Test.Step = TEST_FIN_INFLA_P0;}    // MOD_V0010
				else								{Test.Step = TEST_INFLA_P0;}
				check_comutest(LOW);
				Test.Time = millis();
			}
			break;
		case TEST_INFLA_NOK_P0:
			if (	(Bouton[Bp_IdTest].State == 0)
				&&	(TempsSup(Test.Time, TDef500ms)))
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
			break;
		case TEST_FIN_INFLA_P0_2:
			if (Bouton[Bp_IdTest].State == 0)
			{
				Test.Step = TEST_WAIT_3;

				Test.Time = millis();
			}
			break;
		case TEST_WAIT_3:
			if (TempsSup(Test.Time, TDef1sec))
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


