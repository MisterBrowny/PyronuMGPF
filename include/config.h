#ifndef CONFIG_H
#define	CONFIG_H


#define PAUSE_VALUE         99  // MOD_V0010: valeur de sortie pour activer la pause
#define NB_PAUSE_MAX        5   // MOD_V0010: nb de pause max dans la séquence // TODO vérifier si 5 pause c'est toujours ok
#define	NB_RELAY			16
#define	CF_SECTOR_SIZE		3
#define	CF_CONTROL_SIZE		5
#define	CF_SIZE				(NB_RELAY * CF_SECTOR_SIZE + NB_PAUSE_MAX * CF_SECTOR_SIZE + CF_CONTROL_SIZE) // MOD_V0010

#define OFFSET_LAST_OUTPUT  (NB_RELAY * CF_SECTOR_SIZE + NB_PAUSE_MAX * CF_SECTOR_SIZE)
#define OFFSET_CHECKSUM_1   (OFFSET_LAST_OUTPUT + 1)
#define OFFSET_CHECKSUM_2   (OFFSET_LAST_OUTPUT + 2)
#define OFFSET_CHECKSUM_3   (OFFSET_LAST_OUTPUT + 3)
#define OFFSET_CHECKSUM_4   (OFFSET_LAST_OUTPUT + 4)

typedef struct StructConfig {
	uint8_t					Data[CF_SIZE]; // 16 * 3  + 5 * 3 + 5 // MOD_V0010
	volatile uint8_t		Index;
	volatile unsigned long	Time1;
	unsigned long 			Time2;
	unsigned				IsLong :1;			
}struConfig;

extern struConfig	Cf;

uint8_t cf_check (void);
void cf_check_and_display (void);
void cf_rcv (void);


#endif	/* CONFIG_H */

