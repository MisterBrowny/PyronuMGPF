#ifndef ARMEMENT_H
#define	ARMEMENT_H

// Etape de l'armement
#define ARM_ALIM_1A			0x00
#define ARM_WAIT			0x01
#define ARM_WAIT_1			0x02
#define ARM_WAIT_2			0x03
#define ARM_WAIT_3			0x04
#define ARM_END				0x05

typedef struct	StructArm{
	uint8_t			Step;
	uint8_t			Cpt;
	unsigned long	Time;
	float			U_Alim_1A;
}struArm;

extern struArm Arm;

void armement_process (void);

#endif	/* ARMEMENT_H */

