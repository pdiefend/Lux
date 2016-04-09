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
#define BRIGHTNESS 50

// Globals
Adafruit_NeoPixel topStrip = Adafruit_NeoPixel(nLEDs, TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bottomStrip = Adafruit_NeoPixel(nLEDs, BOTTOM, NEO_GRB + NEO_KHZ800);

boolean exec = false;

void setup() {
  topStrip.begin();
  bottomStrip.begin();
  topStrip.show();
  bottomStrip.show();
  topStrip.setBrightness(BRIGHTNESS);
  bottomStrip.setBrightness(BRIGHTNESS);

  pinMode(MODE_INT, INPUT);
  attachInterrupt(1, modeChange, FALLING);
}

void loop() {
  /*
  if (exec)
   colorWipe(topStrip.Color(255, 0, 0), 50);
   if (exec)
   colorWipe(topStrip.Color(255, 255, 0), 50);
   if (exec)
   colorWipe(topStrip.Color(0, 255, 0), 50);
   if (exec)
   colorWipe(topStrip.Color(0, 255, 255), 50);
   if (exec)
   colorWipe(topStrip.Color(0, 0, 255), 50);
   if (exec)
   colorWipe(topStrip.Color(255, 0, 255), 50);
   if (exec)
   colorWipe(topStrip.Color(255, 255, 255), 50);
   if(exec)
   rainbowCycle(5);
   */
   
  // put your main code here, to run repeatedly:
  //colorWipe(topStrip.Color(0, 0, 0), 10);
  topStrip.setPixelColor(0, topStrip.Color(255,0,0));
  topStrip.show();
  bottomStrip.setPixelColor(0, bottomStrip.Color(255,0,0));
  bottomStrip.show();
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);

  if (digitalRead(BATT_STAT) == HIGH) { // if stat is high, charge complete
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
  }
  delay(1900);
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

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i = 0; i < nLEDs; i++) {
      topStrip.setPixelColor(i, Wheel(((i * 256 / nLEDs) + j) & 255));
      bottomStrip.setPixelColor(i, Wheel(((i * 256 / nLEDs) + j) & 255));
    }
    topStrip.show();
    bottomStrip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return topStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return topStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return topStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void modeChange() {
  exec = !exec;
}


