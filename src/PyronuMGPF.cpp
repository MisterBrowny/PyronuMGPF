#include "includes.h"

// Désactive le watchdog en cas de pb car il y a des while(true)
//#include "soc/rtc_wdt.h"

void setup() {
	// put your setup code here, to run once:

  // Désactive le watchdog en cas de pb  car il y a des while(true)
  //rtc_wdt_protect_off();
  //rtc_wdt_disable();

	// Init les entrées / sorties
	hw_config();

  // Init register
  register_init();

  // Init pixel
  pixel_init();
  // Init ecran st7567s
  st7567s_init();
  
  // Init chaine de caractéres à afficher
  ecran_init();
  
  Micro.Time = millis();

  // Vérifie état COMU_TEST
	check_comutest(LOW);

	// Vérifie si ID_TEST et BP_ON appuyé pour entrer en mode program_0
	if (check_program_0() == false)
	{
		// Vérifie si ID_TEST est appuyé pour entrer en mode programmation
		check_idtest();

		// Vérifie si BP_ON est appuyé
		check_bpon();

		// Vérifie que la config est bonne
		cf_check_and_display();
	}

	bouton_init();
}

//*********************************************************************************//
// LES INTERRUPTIONS
//*********************************************************************************//

// // TODO RS: récupérer les data sur l'UART0
// void interrupt low_priority Low_priority (void)
// {
// 	if (RCIE && RCIF)
// 	{
// 		Cf.Data[Cf.Index] = RCREG;
// 		TXREG = Cf.Data[Cf.Index];

// 		Cf.Index ++;

// 		Cf.Time1 = Cptms;

// 		return;
// 	}
// }

// TODO TIMER attention au Decompte/Cpt1Sur20s
// void interrupt high_priority High_priority (void)
// {
// 	if (TMR0IE && TMR0IF)
// 	{
// 		TMR0IF = 0;

// 		TMR0H = 0xF0;
// 		TMR0L = 0x8C;

// 		Cptms ++;

// 		if (-- Decompte == 0)
// 		{
// 			Decompte = 50;
// 			Cpt1Sur20s ++;
// 		}

// 		if (Micro.State)	// ARMED / GO / STOP / END
// 		{
// 			if (Feedback.Period == 0)
// 			{
// 				Feedback.State = Micro.State;		// Rafraichit l'état du feedback toute les périodes 
// 				Feedback.Period = FEEDBACK_PERIOD;
// 				Feedback.Step = 0;
// 			}
// 			else
// 			{
// 				Feedback.Period --;
// 			}
			
// 			if ((0x01 << (byte) (Feedback.Step / 2)) <= Feedback.State)	// le nb de pulse est fct de l'état ARMED / GO / STOP / END
// 			{
// 				if ((Feedback.Step & 0x01) == 0)
// 				{// PULSE 1, 2, 3, 4
// 					if ((Feedback.Step == 0) || (-- Feedback.Pulse == 0))
// 					{
// 						INFO_OUT = 1;
// 						Feedback.Pulse = FEEDBACK_PULSE;
// 						Feedback.Step ++;
// 					}
// 				}
// 				else
// 				{// INTER-PULSE
// 					if (-- Feedback.Pulse == 0)
// 					{
// 						INFO_OUT = 0;
// 						Feedback.Pulse = FEEDBACK_INTER_PULSE;
// 						Feedback.Step ++;
// 					}
// 				}
// 			}
// 		}
// 		else
// 		{
// 			INFO_OUT = 0;
// 			Feedback.Period = 0;	
// 		}
		
// 		return;
// 	}
// }

//*********************************************************************************//
// MAIN LOOP
//*********************************************************************************//

void loop() {
  // put your main code here, to run repeatedly:
  bouton_refresh();
  st7567s_refresh();

  switch (Micro.Phase)
  {
    case MICRO_WAIT:
      micro_wait();
      break;
    case MICRO_TEST:
      test_process();
      break;
    case MICRO_ARM:
      armement_process();
      break;
    case MICRO_FEU:
      feu_process();
      feu_check_bp();
      break;
  }
}
