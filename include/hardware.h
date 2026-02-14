#ifndef HARDWARE_H
#define	HARDWARE_H


// ENTREES 
#define BP_TEST             4   // GPIO4    / pin7          / NumPin bouton BP_TEST (anciennement ID_TEST) à pull up
#define ID_TEST             digitalRead(BP_TEST)            // Etat bouton BP_TEST (anciennement ID_TEST)

#define COMMUT_PUISS_OUI    5   // GPIO5    / pin10         / NumPin bouton COMMUT_PUIS ACTIF à pull up
#define COMU_PUISS_OUI      digitalRead(COMMUT_PUISS_OUI)   // Etat entrée COMMUT_PUISS_OUI (anciennement COMU_TEST)
#define COMMUT_PUISS_NON    18  // GPIO18   / pin11         / NumPin bouton COMMUT_PUIS NON ACTIF à pull up
#define COMU_PUISS_NON      digitalRead(COMMUT_PUISS_NON)   // Etat entrée COMMUT_PUISS_NON (anciennement COMU_TEST)

#define B_START             19  // GPIO19   / pin12         / NumPin Start (L2+/L2-)(1=Pas Start / 0=Start) pull up externe
#define START               digitalRead(B_START)            // Etat bouton B_START (anciennement START) 

#define BUTTON_ON           34  // GPIO34   / pin24         / NumPin bouton BUTTON_ON (anciennement BP_ON) à pull up en externe
#define BP_ON               digitalRead(BUTTON_ON)          // Etat bouton BUTTON_ON (anciennement BP_ON) 

#define NB_AT               35  // GPIO35   / pin25         / NumPin AT (L1+/L1-)(1=AT / 0=pas AT) à pull down
#define AT_SIGNAL           digitalRead(NB_AT)              // Etat AT (1=AT / 0=pas AT)


// ENTREES ANALOG
#define U_TEST_INF          36  // GPIO36   / pin22         / ADC00, lecture de la tension infla
#define U_TEST_1A           39  // GPIO39   / pin23         / ADC03, lecture de la tension à 1A          

#define U_TEST_INF_ADC      A0
#define U_TEST_1A_ADC       A3

// SORTIES
#define LED_RGB             13  // GPIO13   / pin34     / NumPin WS2812B
#define LED_BPTEST          15  // GPIO15   / pin4      / NumPin led BP_TEST
#define TXS0108_OE          23  // GPIO23   / pin18     / NumPin l'Output Enable de buffer 5V/3.3V TXS0108
#define SECU_PUISSANCE      25  // GPIO25   / pin28     / NumPin la puissance pour le tir (ancien TIR/VERROU_TIR)
#define LOAD_TEST_1A        32  // GPIO32   / pin26     / NumPin le test 1A
#define LOAD_TEST_20mA      33  // GPIO33   / pin27     / NumPin le test infla


// REGISTRE A DECALAGE
#define SER_D               14  // GPIO14   / pin31     / NumPin data/SER 74HC595 = REGIST_SERIE
#define SER_C               26  // GPIO26   / pin29     / NumPin serial data clock/SRCLK 74HC595 = REGIST_SRCLK
#define LOAD_LED            27  // GPIO27   / pin30     / NumPin register clock/RCLK 74HC595 = REGIST_RCLK

// I2C
#define SDA_LCD             21  // GPIO21   / pin14     / NumPin interface avec l'écran st7567s
#define SCL_LCD             22  // GPIO22   / pin17     / NumPin interface avec l'écran st7567s

// ANALOG
#define CONVERSION_ADC	    0.0032258f		// 3.3 / 1023

// Valeur pour récupérer la tension d'alimentation
#define PONT_DIVISEUR	    10.93f		    // ((51000/2) + (100000+150000+3300)) / (51000/2)

// Valeur pour le test U_INFLA 
#define DefValInflaOK	    94			    // 3.3 x (48 / (48 + 120))
                                            // Correspond d'après mesure à 48ohms

// En dessous OK 
// 36 ohm pour 20mA = 0.72 Volts > acceptable => clignote
// 50 ohm pour 20mA = 1 Volt > KO 


/*
// ANCIEN CODE
// ENTREES  
#define BP_ON		PORTEbits.RE0
#define ID_TEST		PORTEbits.RE1
#define COMU_TEST	PORTCbits.RC5
#define START		PORTBbits.RB0

// SORTIES
#define ACTI_MAX	PORTEbits.RE2
#define TIR			PORTBbits.RB1
#define VERROU_TIR	PORTAbits.RA5
*/

void hw_config(void);

#endif	/* HARDWARE_H */

