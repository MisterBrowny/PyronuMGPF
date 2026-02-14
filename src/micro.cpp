#include "includes.h"

struMicro Micro;

void micro_wait (void)
{
	switch (Micro.Step)
	{
		case MICRO_STEP_1:
			if (Bouton[Bp_IdTest].State == 0)
			{
				Micro.Step = MICRO_STEP_2;
				ecran_wait();
			}
			break;
		case MICRO_STEP_2:
			if (Bouton[Bp_IdTest].State == 1)
			{
				Micro.Step = MICRO_STEP_3;
			}
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