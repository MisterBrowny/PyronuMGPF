#include "includes.h"

struEcran	Ecran;

void ecran_init (void)
{
	byte i = 0;

	memset((void*) Ecran.Digit, 0, 7);
	memset((void*) Ecran.MemoDigit, 0, 7);

	Ecran.Digits = Ecran.Digit;
}

void ecran_print_one_char (byte Digit, char Character)
{
	ecran_blank();

	Ecran.Digit[Digit] = Character;
}

void ecran_print_num (byte Num)
{
	char temp_char[5];
	
	DecToStr(Num, &temp_char[0]);

	Ecran.Digit[0] = ' ';
	Ecran.Digit[1] = ' ';
	Ecran.Digit[2] = ' ';

	if (Num < 10)
	{
		Ecran.Digit[4] = temp_char[0];
		Ecran.Digit[3] = '0';
	}
	else
	{
		Ecran.Digit[4] = temp_char[1];
		Ecran.Digit[3] = temp_char[0];
	}
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_wait (void)
{
	Ecran.Digit[0] = 'w';
	Ecran.Digit[1] = 'a';
	Ecran.Digit[2] = 'i';
	Ecran.Digit[3] = 't';
	Ecran.Digit[4] = 0;
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_blank (void)
{
	// Clear l'afficheur
	Ecran.Digit[0] = ' ';
	Ecran.Digit[1] = ' ';
	Ecran.Digit[2] = ' ';
	Ecran.Digit[3] = ' ';
	Ecran.Digit[4] = ' ';
	Ecran.Digit[5] = ' ';
	Ecran.Digit[6] = 0;
}

void ecran_erreur_tir (void)
{
	Ecran.Digit[0] = 'E';
	Ecran.Digit[1] = '_';
	Ecran.Digit[2] = 't';
	Ecran.Digit[3] = ' ';
	Ecran.Digit[4] = ' ';
	Ecran.Digit[5] = ' ';
	Ecran.Digit[6] = 0;
}

void ecran_com_on (void)
{
	Ecran.Digit[0] = 'C';
	Ecran.Digit[1] = 'O';
	Ecran.Digit[2] = 'M';
	Ecran.Digit[3] = '_';
	Ecran.Digit[4] = 'o';
	Ecran.Digit[5] = 'n';
	Ecran.Digit[6] = 0;
}

void ecran_erreur_comu (void)
{
	Ecran.Digit[0] = 'E';
	Ecran.Digit[1] = '_';
	Ecran.Digit[2] = 'c';
	Ecran.Digit[3] = 'o';
	Ecran.Digit[4] = 'm';
	Ecran.Digit[5] = 'u';
	Ecran.Digit[6] = 0;
}

void ecran_erreur_conf (void)
{
	Ecran.Digit[0] = 'E';
	Ecran.Digit[1] = '_';
	Ecran.Digit[2] = 'c';
	Ecran.Digit[3] = 'o';
	Ecran.Digit[4] = 'n';
	Ecran.Digit[5] = 'f';
	Ecran.Digit[6] = 0;
}

void ecran_erreur_eepr (void)
{
	Ecran.Digit[0] = 'E';
	Ecran.Digit[1] = '_';
	Ecran.Digit[2] = 'e';
	Ecran.Digit[3] = 'e';
	Ecran.Digit[4] = 'p';
	Ecran.Digit[5] = 'r';
	Ecran.Digit[6] = 0;
}

void ecran_write_ok (void)
{
	Ecran.Digit[0] = 'W';
	Ecran.Digit[1] = 'r';
	Ecran.Digit[2] = '_';
	Ecran.Digit[3] = 'O';
	Ecran.Digit[4] = 'K';
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_armed (void)
{
	Ecran.Digit[0] = 'A';
	Ecran.Digit[1] = 'r';
	Ecran.Digit[2] = 'm';
	Ecran.Digit[3] = 'e';
	Ecran.Digit[4] = 'd';
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_go (void)
{
	Ecran.Digit[0] = 'G';
	Ecran.Digit[1] = 'O';
	Ecran.Digit[2] = 0;
	Ecran.Digit[3] = 0;
	Ecran.Digit[4] = 0;
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_stop (void)
{
	Ecran.Digit[0] = 'S';
	Ecran.Digit[1] = 't';
	Ecran.Digit[2] = 'o';
	Ecran.Digit[3] = 'p';
	Ecran.Digit[4] = 0;
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_end (void)
{
	Ecran.Digit[0] = 'E';
	Ecran.Digit[1] = 'n';
	Ecran.Digit[2] = 'd';
	Ecran.Digit[3] = 0;
	Ecran.Digit[4] = 0;
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}

void ecran_bstart (void)
{
	Ecran.Digit[0] = 'B';
	Ecran.Digit[1] = 's';
	Ecran.Digit[2] = 't';
	Ecran.Digit[3] = 'a';
	Ecran.Digit[4] = 'r';
	Ecran.Digit[5] = 't';
	Ecran.Digit[6] = 0;
}

void ecran_prog (void)
{
	Ecran.Digit[0] = 'P';
	Ecran.Digit[1] = 'r';
	Ecran.Digit[2] = 'o';
	Ecran.Digit[3] = 'g';
	Ecran.Digit[4] = 0;
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}


void ecran_prog_0 (void)
{
	Ecran.Digit[0] = 'P';
	Ecran.Digit[1] = 'r';
	Ecran.Digit[2] = 'o';
	Ecran.Digit[3] = 'g';
	Ecran.Digit[4] = '_';
	Ecran.Digit[5] = '0';
	Ecran.Digit[6] = 0;
}

void ecran_bp_on (void)
{
	Ecran.Digit[0] = 'B';
	Ecran.Digit[1] = 'P';
	Ecran.Digit[2] = '_';
	Ecran.Digit[3] = 'O';
	Ecran.Digit[4] = 'N';
	Ecran.Digit[5] = 0;
	Ecran.Digit[6] = 0;
}