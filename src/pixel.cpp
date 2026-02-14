#include "includes.h"

#include "Adafruit_NeoPixel.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        	LED_RGB

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 	5

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void pixel_init (void)
{
	SERIAL_DEBUG("Pixel config begins");
	
	pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

	pixels.clear(); // Set all pixel colors to 'off'
	pixels.show();

	pixel_set_default(); // Set all pixel default colors (RED  / GREEN / BLUE)
	
	SERIAL_DEBUG("Pixel config ends");
}

void pixel_set_default (void)
{
	// pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(0, pixels.Color(0, 0, 200));
	pixels.setPixelColor(1, pixels.Color(0, 0, 200));
	// pixels.setPixelColor(2, pixels.Color(0, 255, 0));
	// pixels.setPixelColor(3, pixels.Color(0, 255, 0));
	// pixels.setPixelColor(4, pixels.Color(0, 255, 0));
    pixels.show();   // Send the updated pixel colors to the hardware.
}

void pixel_set (uint8_t numpixel, uint8_t red, uint8_t green, uint8_t blue)
{
	// pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(numpixel, pixels.Color(red, green, blue));

    pixels.show();   // Send the updated pixel colors to the hardware.
}