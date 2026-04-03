#include "includes.h"


const uint8_t SLAVE_ID = 20;       // ← CHANGE CE NUMÉRO pour chaque ESP32 (1 à 20)

// Désactive le watchdog en cas de pb car il y a des while(true)
//#include "soc/rtc_wdt.h"

void setup() {
	// put your setup code here, to run once:
  
  // Désactive le watchdog en cas de pb  car il y a des while(true)
  //rtc_wdt_protect_off();
  //rtc_wdt_disable();

	// Init les entrées / sorties
	hw_config();

  // Init ecran st7567s
  st7567s_init();

  // Init register
  register_init();

  // Init EEPROM
  eeprom_init();
  
  // Init pixel
  pixel_init();
    
  // Init chaine de caractéres à afficher
  ecran_init();
  
  Micro.Time = millis();

  // Init modbus
  modbus_init();

	// Vérifie si ID_TEST et BP_ON appuyé pour entrer en mode program_0
	if (check_program_0() == false)
	{
		// Vérifie si ID_TEST est appuyé pour entrer en mode programmation
		check_idtest();

		// Vérifie que la config est bonne
		cf_check_and_display();
	}

	bouton_init();
}

//*********************************************************************************//
// MAIN LOOP
//*********************************************************************************//

void loop() {
  // put your main code here, to run repeatedly:
  bouton_refresh();
  st7567s_refresh();
  modbus_refresh();
  
  switch (Micro.Phase)
  {
    case MICRO_WAIT:
      micro_wait();
      break;
    case MICRO_TEST:
      test_process();
      break;
    case MICRO_ARM:
      armement_process();
      break;
    case MICRO_FEU:
      feu_process();
      feu_check_bp();
      break;
  }
}
