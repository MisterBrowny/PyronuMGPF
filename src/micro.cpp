#include "includes.h"

struMicro Micro;

void micro_wait (void)
{
	switch (Micro.Step)
	{
		case MICRO_STEP_1:
			if (	(Bouton[Bp_IdTest].State == 0)
				||	(TempsSup(Micro.Time, TDef5sec)))
			{
				Micro.Step = MICRO_STEP_2;
				
  				// Vérifie état COMU_TEST
				check_comutest(LOW);
			}
			break;
		case MICRO_STEP_2:
			Micro.Phase = MICRO_TEST;
			Test.Step = TEST_WAIT;
			check_comutest(LOW);
			check_bpon();
		
			//ecran_blank();
			break;
		case MICRO_STEP_3:
			if (Bouton[Bp_IdTest].State == 0)
			{
				Micro.Phase = MICRO_TEST;
				Test.Step = TEST_WAIT;
				check_comutest(LOW);
				ecran_blank();
			}
			else if (Bouton[Bp_On].State == 0)
			{
				Micro.Phase = MICRO_ARM;
				Arm.Step = ARM_WAIT;
				check_comutest(HIGH);
				ecran_blank();
			}

			if (Bouton[Bp_Start].State == 0)
			{
				word tempTime = millis();
				
				ecran_blank();
				
				ecran_bstart();
				st7567s_refresh();
				
				Micro.State = END;
				
				while (START == 0)
				{
					if (TempsSup(tempTime, TDef1sec))
					{
						tempTime = millis();
						Micro.State = ((Micro.State == END) ? UNDEFINED : END);
					}
				};
				
				Micro.State = UNDEFINED;
				
				ecran_wait();
			}
			break;
	}
}