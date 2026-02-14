#ifndef INCLUDE_H
#define	INCLUDE_H

//// XC8 LIBRAIRIES
//#include "pic18f452.h"
//#include "stdio.h"
//#include "stdlib.h"

#include <Arduino.h>
#include <stdint.h>

// Debug Print
#define DEBUG_PRINT     1   // A mettre à 1 pour activer l'envoi de log sur la liaison série

#if DEBUG_PRINT
	#define SERIAL_DEBUG(x)		        \
        do                              \
        {                               \
           Serial.print(__FILE__);      \
           Serial.print(" l.");         \
           Serial.print(__LINE__);      \
           Serial.print(" (");          \
           Serial.print(__FUNCTION__);  \
           Serial.print("): ");         \
           Serial.println(x);           \
        }                               \
        while (0)
#else
	#define SERIAL_DEBUG(x)
#endif


#include "constant.h"
#include "utils.h"
#include "micro.h"
#include "hardware.h"
#include "eeprom.h"

#include "timer.h"
#include "bouton.h"
#include "config.h"
#include "ecran.h"
#include "register.h"
#include "test.h"
#include "armement.h"
#include "feu.h"
#include "feedback.h"

#include "pixel.h"
#include "st7567s.h"


#endif	/* INCLUDE_H */

