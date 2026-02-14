#include "includes.h"



void hw_config (void)
{
	// Liaison série debug/download
	Serial.begin(115200);

	SERIAL_DEBUG("Hardware config begins");

	// Liaison série RS485
	Serial2.begin(9600); // TODO Speed/conf a vérifier

	// ENTREES
	pinMode(NB_AT, INPUT_PULLDOWN);
	pinMode(BP_TEST, INPUT_PULLUP);				
	pinMode(COMMUT_PUISS_OUI, INPUT_PULLUP);
	pinMode(COMMUT_PUISS_NON, INPUT_PULLUP);
	pinMode(B_START, INPUT);
	pinMode(BUTTON_ON, INPUT);

	// ENTREES ANALOG
	pinMode(U_TEST_INF, INPUT); // A0
	pinMode(U_TEST_1A, INPUT); 	// A3

	// SORTIES
	// pilote led BP_TEST
	pinMode(LED_BPTEST, OUTPUT);
	digitalWrite(LED_BPTEST, HIGH);

	// pilote l'Output Enable de buffer 5V/3.3V TXS0108
	pinMode(TXS0108_OE, OUTPUT);
	digitalWrite(TXS0108_OE, HIGH);

	// pilote la puissance pour le tir (ancien TIR/VERROU_TIR)
	pinMode(SECU_PUISSANCE, OUTPUT);
	digitalWrite(SECU_PUISSANCE, LOW);

	// pilote le test 1A
	pinMode(LOAD_TEST_1A, OUTPUT);
	digitalWrite(LOAD_TEST_1A, LOW);

	// pilote le test infla
	pinMode(LOAD_TEST_20mA, OUTPUT);
	digitalWrite(LOAD_TEST_20mA, LOW);

	// REGISTRE A DECALAGE
	// data/SER 74HC595 = REGIST_SERIE
	pinMode(SER_D, OUTPUT);
	digitalWrite(SER_D, LOW);

	// serial data clock/SRCLK 74HC595 = REGIST_SRCLK
	pinMode(SER_C, OUTPUT);
	digitalWrite(SER_C, LOW);

	// register clock/RCLK 74HC595 = REGIST_RCLK
	pinMode(LOAD_LED, OUTPUT);
	digitalWrite(LOAD_LED, LOW);

	SERIAL_DEBUG("Hardware config ends");
}
