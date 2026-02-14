#ifndef BOUTON_H
#define	BOUTON_H

enum Boutons {Bp_On, Bp_IdTest, /*Comu_Test,*/ Bp_Start, Bp_Max};

typedef struct	StructBouton{
	unsigned long		Time;
	unsigned			Memo	:1;
	unsigned			State	:1;
	unsigned			Change	:1;
}struBouton;

extern struBouton	Bouton[Bp_Max];

void bouton_init (void);
void bouton_refresh (void);

#endif	/* BOUTON_H */

