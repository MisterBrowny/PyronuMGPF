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
#define TEST_FIN_INFLA_3		0x0A
#define TEST_INFLA_P0			0x10
#define TEST_INFLA_2_P0			0x11
#define TEST_INFLA_OK_P0		0x12
#define TEST_INFLA_NOK_P0		0x13
#define TEST_FIN_INFLA_P0		0x14
#define TEST_FIN_INFLA_P0_2		0x15
#define TEST_FIN_INFLA_P0_3		0x16
#define TEST_WAIT_3				0x20
#define TEST_WAIT_4				0x21
#define TEST_WAIT_5				0x22
#define TEST_WAIT_6				0x23
#define TEST_WAIT_7				0x24

#define TEST_NO_INFLA_PRINT		0x30    // MOD_V0010
#define TEST_NO_INFLA_PAUSE		0x31    // MOD_V0010

#define TEST_PRINT_RESULT		0x40

// Led Print Test Status
#define TEST_LED_TIMING			TDef50ms
#define TEST_LED_CNT_MAX		20
#define TEST_LED_KO_FLASH		20
#define TEST_LED_MOYEN_BLINK	10

typedef struct	StructTest{
	uint8_t			Step;
	uint8_t			Cpt;
	uint8_t			Led_process_cnt;
	unsigned long	Time;
	unsigned long	Led_process_time;
	float			U_Alim;
	float			U_Infla;
	bool			print_result;
	bool			no_display_refresh;
}struTest;

extern struTest Test;

void check_comutest(uint8_t State);
uint8_t check_program_0 (void);
void check_idtest (void);
void check_bpon (void);
uint8_t test_process (void);


#endif	/* TEST_H */

