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
				digitalWrite(LED_BPTEST, HIGH);
				digitalWrite(LED_BPON, LOW);
				Micro.Phase = MICRO_TEST;
				Test.Step = TEST_WAIT;
				check_comutest(LOW);
				ecran_blank();
			}
			else if (Bouton[Bp_On].State == 0)
			{
				digitalWrite(LED_BPTEST, LOW);
				digitalWrite(LED_BPON, HIGH);
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
			else
			{
				if (TempsSup(Micro.Time, TDef500ms))
				{
					Micro.Time = millis();
					Micro.etat_led_bouton ^= true;
						
					if (COMU_PUISS_OUI == LOW)
					{// Si PUISSANCE ON => c'est le bouton BP_ON qui clignote
						if (Micro.memo_comu != COMU_PUISSANCE_ON)
						{// Allume direct la led si changement d'état
							Micro.etat_led_bouton = true;
						}

						digitalWrite(LED_BPTEST, LOW);
						digitalWrite(LED_BPON, (Micro.etat_led_bouton ? LOW : HIGH));
						Micro.memo_comu = COMU_PUISSANCE_ON; // COMU_PUISS_OUI actif
					}
					else if (COMU_PUISS_NON == LOW)
					{// Si PUISSANCE OFF => c'est le bouton BP_TEST qui clignote
						if (Micro.memo_comu != COMU_PUISSANCE_OFF)
						{// Allume direct la led si changement d'état
							Micro.etat_led_bouton = true;
						}
						
						digitalWrite(LED_BPTEST, (Micro.etat_led_bouton ? LOW : HIGH));
						digitalWrite(LED_BPON, LOW);
						Micro.memo_comu = COMU_PUISSANCE_OFF; // COMU_PUISS_NON inactif
					}
				}
			}
			break;
	}
}