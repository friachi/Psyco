#include <FastLED.h>
#define NUM_LEDS 64
#define DATA_PIN 10   // Psyco reserved pin D10: typically for LED control

CHSV spectrumcolor = CHSV( 160, 255, 60);
CRGB leds[NUM_LEDS];

 void setup() {
       FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
       FastLED.clear();
   }

void loop() {
   leds[9] = spectrumcolor;
   leds[11] = spectrumcolor;

   leds[16] = spectrumcolor;
   leds[18] = spectrumcolor;
   leds[20] = spectrumcolor;
   leds[22] = spectrumcolor;

   leds[33] = spectrumcolor;
   leds[35] = spectrumcolor;
   leds[37] = spectrumcolor;

   leds[40] = spectrumcolor;
   leds[42] = spectrumcolor;
   leds[44] = spectrumcolor;
   leds[46] = spectrumcolor;

   FastLED.show();

   delay(50);
 }