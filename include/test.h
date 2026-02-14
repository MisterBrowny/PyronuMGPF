#ifndef TEST_H
#define	TEST_H



// Etape du test initial
#define TEST_INIT				0x00
#define TEST_WAIT				0x01
#define TEST_ALIM				0x02
#define TEST_WAIT_2				0x03
#define TEST_INFLA				0x04
#define TEST_INFLA_2			0x05
#define TEST_INFLA_OK			0x06
#define TEST_INFLA_NOK			0x07
#define TEST_FIN_INFLA			0x08
#define TEST_FIN_INFLA_2		0x09
#define TEST_INFLA_P0			0x0A
#define TEST_INFLA_2_P0			0x0B
#define TEST_INFLA_OK_P0		0x0C
#define TEST_INFLA_NOK_P0		0x0D
#define TEST_FIN_INFLA_P0		0x0E
#define TEST_FIN_INFLA_P0_2		0x0F
#define TEST_WAIT_3				0x10
#define TEST_WAIT_4				0x11
#define TEST_WAIT_5				0x12
#define TEST_WAIT_6				0x13
#define TEST_WAIT_7				0x14

#define TEST_NO_INFLA_PRINT		0x15    // MOD_V0010
#define TEST_NO_INFLA_PAUSE		0x16    // MOD_V0010

typedef struct	StructTest{
	uint8_t			Step;
	uint8_t			Cpt;
	unsigned long	Time;
	float			U_Alim;
	float			U_Infla;
	float			Test1;
	float			Test2;
	float			Test3;
	float			Test4;
	float			Test5;
	float			Test6;
	float			Test7;
}struTest;

extern struTest Test;

void check_comutest(uint8_t State);
uint8_t check_program_0 (void);
void check_idtest (void);
void check_bpon (void);
uint8_t test_process (void);


#endif	/* TEST_H */

