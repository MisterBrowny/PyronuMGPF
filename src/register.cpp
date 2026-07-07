#include "includes.h"

#include "ShiftRegister74HC595.h"

//   * @param ser The SER pin number which is used to set the new incoming value.
// #define SER_D               14  // GPIO14   / pin31     / NumPin data/SER 74HC595 = REGIST_SERIE
//   * @param rck The RCK pin number which is used to copy the shift register values to the register for ouptuts.
// #define LOAD_LED            27  // GPIO27   / pin30     / NumPin register clock/RCLK 74HC595 = REGIST_RCLK
//   * @param srck The SRCK pin number which is used to shift to right the shift register.
// #define SER_C               26  // GPIO26   / pin29     / NumPin serial data clock/SRCLK 74HC595 = REGIST_SRCLK
//   * @param numberOfSRegister The number of 74hc595 registers linked together.
// #define REGISTERS_NUMBER 4

ShiftRegister74HC595<REGISTERS_NUMBER> sr(SER_D, SER_C, LOAD_LED);

const uint8_t Led[REGISTER_LED_NUMBER] = 
{0, 2, 4, 6, 8, 10, 12, 14, 30, 28, 26, 24, 22, 20, 18, 16};
// {0, 30, 2, 28, 4, 26, 6, 24, 8, 22, 10, 20, 12, 18, 14, 16};

const uint8_t Tir[REGISTER_TIR_OUT_NUMBER] = 
{1, 3, 5, 7, 9, 11, 13, 15, 31, 29, 27, 25, 23, 21, 19, 17};
// {1, 31, 3, 29, 5, 27, 7, 25, 9, 23, 11, 21, 13, 19, 15, 17};

void register_init (void)
{
	uint8_t led = 0;

	SERIAL_DEBUG("Register init begins");
	
	//while(1)
	{
		register_raz();

		for (led = 1; led <= REGISTER_LED_NUMBER; led ++)
		{
			register_one_led_on(led);
			delay(100);
		}

		register_raz();
	}

	SERIAL_DEBUG("Register init end");
}

//  TODO MODIFIE LA FCT POUR LED OU TIR
void register_write_one_high (uint8_t Output)
{
	register_raz();
	sr.set(Output, HIGH);
}

void register_one_led_on (uint8_t led)
{
	register_write_one_high(Led[led-1]);
}

void register_one_tir_on (uint8_t tir)
{
	register_write_one_high(Tir[tir-1]);
}

void register_raz (void)
{
	sr.setAllLow(); // set all pins LOW
}


void register_print_test_status (void)
{
	uint8_t i;

	if (TempsSup(Test.Led_process_time, TDef50ms))
	{
		Test.Led_process_time = millis();
		if (++ Test.Led_process_cnt > TEST_LED_CNT_MAX)
		{
			Test.Led_process_cnt = 0;
		}

		for (i = 0; i < NB_TIR; i ++)
		{
			if (((Modbus.state.analog[((i < 8) ? 0 : 1)] >> ((i < 8) ? i : i - 8)*2) & MASK_ANALOG_STATE) == ANALOG_OK)
			{        
				sr.setNoUpdate(Led[i], HIGH);
			}
			else if (((Modbus.state.analog[((i < 8) ? 0 : 1)] >> ((i < 8) ? i : i - 8)*2) & MASK_ANALOG_STATE) == ANALOG_MOYEN)
			{
				if (Test.Led_process_cnt < TEST_LED_MOYEN_BLINK)
				{
					sr.setNoUpdate(Led[i], HIGH);
				}
				else
				{
					sr.setNoUpdate(Led[i], LOW);
				}
			}
			else if (((Modbus.state.analog[((i < 8) ? 0 : 1)] >> ((i < 8) ? i : i - 8)*2) & MASK_ANALOG_STATE) == ANALOG_KO)
			{
				if (Test.Led_process_cnt == TEST_LED_KO_FLASH)
				{
					sr.setNoUpdate(Led[i], HIGH);
				}
				else
				{
					sr.setNoUpdate(Led[i], LOW);
				}
			}
			else
			{
				sr.setNoUpdate(Led[i], LOW);
			}
		}
		sr.updateRegisters();
	}    
}

// void loop() {

//   // setting all pins at the same time to either HIGH or LOW
//   sr.setAllHigh(); // set all pins HIGH
//   delay(500);
  
//   sr.setAllLow(); // set all pins LOW
//   delay(500); 
  

//   // setting single pins
//   for (int i = 0; i < 8; i++) {
    
//     sr.set(i, HIGH); // set single pin HIGH
//     delay(250); 
//   }
  
  
//   // set all pins at once
//   uint8_t pinValues[] = { B10101010 }; 
//   sr.setAll(pinValues); 
//   delay(1000);

  
//   // read pin (zero based, i.e. 6th pin)
//   uint8_t stateOfPin5 = sr.get(5);
//   sr.set(6, stateOfPin5);


//   // set pins without immediate update
//   sr.setNoUpdate(0, HIGH);
//   sr.setNoUpdate(1, LOW);
//   // at this point of time, pin 0 and 1 did not change yet
//   sr.updateRegisters(); // update the pins to the set values
// }


// TODO il y a seulement 16 sorties avec a chaque sortie une led associée
// void register_write (byte Output)
// {
// 	byte i;

// 	// Désactive la gachette
// 	//REGIST_G = 0;

// 	// clear le registre
// 	//REGIST_SRCLR = 0;
// 	//REGIST_SRCLR = 1;

// 	digitalWrite(LOAD_LED, LOW);	// REGIST_RCLK = 0;

// 	for (i = 32; i > 0; i --)
// 	{
// 		digitalWrite(SER_C, LOW); 	// REGIST_SRCLK = 0;
		
// 		if (Output == i)	{digitalWrite(SER_C, HIGH);}	// {REGIST_SERIE = 1;}
// 		else				{digitalWrite(SER_C, LOW);}		// {REGIST_SERIE = 0;}

// 		digitalWrite(SER_C, HIGH); 	// REGIST_SRCLK = 1;
// 	}

// 	digitalWrite(LOAD_LED, LOW); 	// REGIST_RCLK = 1;

// 	// réactive la gachette
// 	//REGIST_G = 1;*/
// }