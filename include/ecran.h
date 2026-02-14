#ifndef ECRAN_H
#define	ECRAN_H

#define NUM_CHAR	7

typedef struct	StructEcran{
	volatile char Digit[NUM_CHAR];
	volatile char MemoDigit[NUM_CHAR];

	volatile char *Digits;
	
	uint8_t Dot[4];
	uint8_t Cpt;
	uint8_t Solid;
}struEcran;

extern struEcran	Ecran;

void ecran_init (void);
void ecran_print_one_char (uint8_t Digit, char Character);
void ecran_print_num (uint8_t Num);
void ecran_wait (void);
void ecran_blank (void);
void ecran_erreur_tir (void);
void ecran_com_on (void);
void ecran_erreur_comu (void);
void ecran_erreur_conf (void);
void ecran_erreur_eepr (void);
void ecran_write_ok (void);
void ecran_armed (void);
void ecran_go (void);
void ecran_stop (void);
void ecran_end (void);
void ecran_bstart (void);
void ecran_prog (void);
void ecran_prog_0 (void);
void ecran_bp_on (void);

#endif	/* ECRAN_H */

