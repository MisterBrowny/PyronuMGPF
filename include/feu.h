#ifndef FEU_H
#define	FEU_H

// Temps activation infla en ms (Attention ce temps ne doit pas dépasser 50ms)
#define DefTempsTir			40

// Temps d'attente minimum pour stopper la séquence automatique en cours
#define TIME_TO_STOP		5000

// Etape du feu d'artifice
#define FEU_ARMED                   0x00
#define FEU_SELECT                  0x01
#define FEU_GO                      0x02
#define FEU_SELECT_P0               0x03
#define FEU_GO_P0                   0x04
#define FEU_WAIT_P0                 0x05
#define FEU_STOP                    0x06
#define FEU_RESTART                 0x07
#define FEU_END                     0x08
#define FEU_PAUSE                   0x09    // MOD_V0010
#define FEU_RESTART_AFTER_PAUSE     0x0A    // MOD_V0010

// volatile uint8_t Decompte;
// volatile uint32_t Cpt1Sur20s;


#define CPT_1_20_S      (unsigned long) ((millis() - Feu.TimeStart) / 50)

typedef struct	StructFeu{
	uint8_t			Step;
	uint8_t			Cpt;
	//uint8_t		Decompte;
	unsigned long	SavedTime;
	unsigned long	TimeToFire;
	unsigned long	Time;
	unsigned long	TimeStart;
	uint8_t			LastOutput;	// derniére sortie tirée
	uint8_t			NextOutput;	// prochaine sortie à tirer
	unsigned		CanStopStart	:1;
	unsigned		SaveTime		:1;
}struFeu;

extern struFeu Feu;

void feu_check_bp (void);
void feu_process (void);

#endif	/* FEU_H */

