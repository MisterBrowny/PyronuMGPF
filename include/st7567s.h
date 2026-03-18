#ifndef ST7567S_H
#define	ST7567S_H

#include "lib\st7567sfGK_128x64_i2c_LCD_driver_for_Generation_Klick\src\st7567sfGK.h"

#include "font/FreeSans9pt7b.h"
#include "font/Picopixel.h"
#include "font/Tiny3x3a2pt7b.h"
#include "font/TomThumb.h"

// Here it is!
extern st7567sfGKAdafruit display;

#define ST_I2C_SPEED	100000

void st7567s_init (void);

void st7567s_refresh (void);

#endif	/* ST7567S_H */

