#include "includes.h"

struArm Arm;

static void arm_UAlim_1A (void)
{// TODO faut-il tester COM_TIR ou NB_TIR ?
	int 	temp = 0;
	char 	temp_tab[5] = {0};

	//VERROU_TIR = 0;
	//TIR = 1;
	digitalWrite(LOAD_TEST_1A, HIGH);

	delay(10);

	temp = analogRead(U_TEST_1A_ADC);

	//VERROU_TIR = 0;
	//TIR = 0;
	digitalWrite(LOAD_TEST_1A, LOW);

	Arm.U_Alim_1A = (float) temp * CONVERSION_ADC;
	Arm.U_Alim_1A = Arm.U_Alim_1A * PONT_DIVISEUR;
	Arm.U_Alim_1A = Arm.U_Alim_1A * 100.0f;

	itoa((int) Arm.U_Alim_1A, &temp_tab[0], 10);

	if (Arm.U_Alim_1A < 1000.0f)
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

void armement_process (void)
{
	word temp;

	switch(Arm.Step)
	{
		case ARM_WAIT:
			if (Bouton[Bp_On].State == 1)
			{
				Arm.Step = ARM_ALIM_1A;
			}
			break;
		case ARM_ALIM_1A:
			check_comutest(HIGH);

			arm_UAlim_1A();

			Arm.Step = ARM_WAIT_1;
			break;
		case ARM_WAIT_1:
			if (Bouton[Bp_On].State == 0)
			{
				Arm.Time = millis();
				Arm.Step = ARM_WAIT_2;

				ecran_blank();
			}
			break;
		case ARM_WAIT_2:
			if (Bouton[Bp_On].State == 1)
			{
				Micro.Phase = MICRO_WAIT;

				ecran_wait();
			}
			else if (Bouton[Bp_Start].State == 0)
			{
				// la ligne de commande est commandée impossible d'armer le module
			}// TODO ajouté vérif NB_AT
			else if (TempsSup(Arm.Time, TDef4sec))
			{
				// la ligne de commande a été commandée entre 3 et 4 secondes d'appuie sur Bp_On le module ne sera pas armé
			}
			else if (TempsSup(Arm.Time, TDef3sec))
			{
				Micro.Phase = MICRO_FEU;
				Micro.State = ARMED;
			}
			break;
	}
}