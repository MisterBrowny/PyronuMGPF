#include "includes.h"

struBouton	Bouton[Bp_Max];

void bouton_init (void)
{
	SERIAL_DEBUG("Bouton init begins");

	Bouton[Bp_On].State = BP_ON;
	Bouton[Bp_IdTest].State = ID_TEST;
	/*Bouton[Comu_Test].State = COMU_TEST;*/
	Bouton[Bp_Start].State = START;

	SERIAL_DEBUG("Bouton init ends");
}

void bouton_refresh (void)
{
	byte i;

	for (i = 0; i < Bp_Max; i++)
	{
		byte temp;
		
		switch (i)
		{
			case Bp_On:		temp = BP_ON;		break;
			case Bp_IdTest:	temp = ID_TEST;		break;
			/*case Comu_Test:	temp = COMU_TEST;	break;*/
			case Bp_Start:	temp = START;		break;
			default:							break;
		}

		if (temp == Bouton[i].Memo)
		{
			if (Bouton[i].State != Bouton[i].Memo)
			{
				if (TempsSup(Bouton[i].Time, (((i == Bp_Start) && (Micro.Mods == true)) ? TDef5ms : TDef100ms)))
				{
					Bouton[i].State = Bouton[i].Memo;
				}
			}
		}
		else
		{
			Bouton[i].Memo = temp;
			Bouton[i].Time = millis();
		}
	}
}



