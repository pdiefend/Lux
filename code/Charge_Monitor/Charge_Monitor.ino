#include <Adafruit_NeoPixel.h>

// Pin definitions
#define ACCEL_INT 2
#define MODE_INT 3
#define LED 4
#define TOP 7
#define BOTTOM 8
#define BATT_STAT 10

// Settings
#define LED 4
#define BATT_STAT 10
#define PRELOAD 59286 // preload timer 65536-16MHz/256/10Hz

#define nLEDs 32
#define BRIGHTNESS 50
#define RAD2DEG (180.0/M_PI)

// Globals
Adafruit_NeoPixel topStrip = Adafruit_NeoPixel(nLEDs, TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bottomStrip = Adafruit_NeoPixel(nLEDs, BOTTOM, NEO_GRB + NEO_KHZ800);


void setup() {
  // put your setup code here, to run once:
  pinMode(BATT_STAT, INPUT);
  pinMode(LED, OUTPUT);
  initTimer();
  
  topStrip.begin();
  bottomStrip.begin();
  for(int i = 0; i<nLEDs; i++){
    topStrip.setPixelColor(i, topStrip.Color(0,0,0));
    bottomStrip.setPixelColor(i, bottomStrip.Color(0,0,0));
  }
  topStrip.show();
  bottomStrip.show();
  topStrip.setBrightness(BRIGHTNESS);
  bottomStrip.setBrightness(BRIGHTNESS);
}

void loop() {

}

void initTimer(){
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = PRELOAD;          // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}

int chargeCounter = 0;

ISR(TIMER1_OVF_vect){
  TCNT1 = PRELOAD;
  chargeCounter++;
  if(chargeCounter == 20){
    chargeCounter = 0;
    digitalWrite(LED, LOW);
  }
  else if (chargeCounter == 19)
    digitalWrite(LED, HIGH);
  else if (chargeCounter == 18)
    digitalWrite(LED, LOW);  
  else if ((chargeCounter == 17) && digitalRead(BATT_STAT))
    digitalWrite(LED, HIGH);
}

