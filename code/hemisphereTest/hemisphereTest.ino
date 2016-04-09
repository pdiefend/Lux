#include <Adafruit_NeoPixel.h>

// Pin definitions
#define ACCEL_INT 2
#define MODE_INT 3
#define LED 4
#define TOP 7
#define BOTTOM 8
#define BATT_STAT 10

// Settings
#define nLEDs 32
#define BRIGHTNESS 125

Adafruit_NeoPixel topStrip = Adafruit_NeoPixel(nLEDs, TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bottomStrip = Adafruit_NeoPixel(nLEDs, BOTTOM, NEO_GRB + NEO_KHZ800);

void setup() {
  topStrip.begin();
  bottomStrip.begin();
  topStrip.show();
  bottomStrip.show();
  topStrip.setBrightness(BRIGHTNESS);
  bottomStrip.setBrightness(BRIGHTNESS);
}

void loop() {
  colorWipe(topStrip.Color(255, 0, 0), 50);
  colorWipe(topStrip.Color(255, 255, 0), 50);
  colorWipe(topStrip.Color(0, 255, 0), 50);
  colorWipe(topStrip.Color(0, 255, 255), 50);
  colorWipe(topStrip.Color(0, 0, 255), 50);
  colorWipe(topStrip.Color(255, 0, 255), 50);
  colorWipe(topStrip.Color(255, 255, 255), 50);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < nLEDs; i++) {
    topStrip.setPixelColor(i, c);
    bottomStrip.setPixelColor(i, c);
    topStrip.show();
    bottomStrip.show();
    delay(wait);
  }
}
