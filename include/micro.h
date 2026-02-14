#ifndef MICRO_H
#define	MICRO_H



// MOD_V0010: trace les évolutions sur la V0010
// Cette version ajoute la fonctionnalité de pause (Etat = STOP) avec la sortie PAUSE_VALUE
// Lorsque le séquenceur est en pause (Etat = STOP) , celui-ci doit être relancé par un appui sur play

// Attention la taille de la config change pour intégrer le nombre de pause max NB_PAUSE_MAX
// voir le fichier 'new sequence v1.3.ods'


// Micro->Phase
#define MICRO_WAIT		0x00
#define MICRO_TEST		0x01
#define MICRO_ARM		0x02
#define MICRO_FEU		0x03

// Micro->Step
#define MICRO_STEP_1	0x00
#define MICRO_STEP_2	0x01
#define MICRO_STEP_3	0x03

// Micro.State
#define UNDEFINED		0x00
#define ARMED			0x01
#define GO				0x02
#define STOP			0x04
#define END				0x08

typedef struct StructState{
	unsigned	Armed	:1;
	unsigned	Go		:1;
	unsigned	Stop	:1;
	unsigned	End		:1;
}StruState;

typedef struct StructMod{
	unsigned	P_0		:1;	// Program qui active successivement les sorties dans l'ordre suivant [1-32]
	unsigned	P_00	:1;	// Program qui active successivement les sorties dans l'ordre souhaité par l'utilisateur
}StruMode;

typedef struct StructMicro {
	unsigned long		Time;
	uint8_t				Phase;
	uint8_t				Step;
	union {
		uint8_t	State;
		struct	StructState	Stat;
	};
	union {
		uint8_t				Mods;
		struct	StructMod	Mod;
	};
}struMicro;

extern struMicro Micro;

void micro_wait (void);


#endif	/* MICRO_H */

