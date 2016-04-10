#include <Wire.h>
//#include <Math.h> // already included elsewhere?
#include <Adafruit_NeoPixel.h>
#include "MMA8452Q.h"

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
#define PRELOAD 59286 // preload timer 65536-16MHz/256/10Hz

// Triggers
// #define CHARGE_MONITOR

// Globals
Adafruit_NeoPixel topStrip = Adafruit_NeoPixel(nLEDs, TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bottomStrip = Adafruit_NeoPixel(nLEDs, BOTTOM, NEO_GRB + NEO_KHZ800);

byte indices[8][8] = {
  {
    31,24,23,16,15,8,7,0                  }
  ,{
    30,25,22,17,14,9,6,1                  }
  ,{
    29,26,21,18,13,10,5,2                  }
  ,{
    28,27,20,19,12,11,4,3                  }
  ,{
    28,27,20,19,12,11,4,3                  }
  ,{
    29,26,21,18,13,10,5,2                  }
  ,{
    30,25,22,17,14,9,6,1                  }
  ,{
    31,24,23,16,15,8,7,0                  }
};
byte red[8][8];
byte blue[8][8];
byte green[8][8];

boolean exec = false;
byte shakeFlag = 0;


volatile byte modeNum = 0;

// ======================================================================================================

void setup() {
  Serial.begin(115200);

  topStrip.begin();
  bottomStrip.begin();
  topStrip.show();
  bottomStrip.show();
  topStrip.setBrightness(BRIGHTNESS);
  bottomStrip.setBrightness(BRIGHTNESS);

  pinMode(MODE_INT, INPUT);
  attachInterrupt(1, modeChange, FALLING);
  pinMode(ACCEL_INT, INPUT);
  attachInterrupt(0, shake, FALLING);

  Wire.begin(); //Join the bus as a master
  Serial.println("Begin");

  initMMA8452(); //Test and intialize the MMA8452
#ifdef CHARGE_MONITOR
  initTimer(); // Start the Timer used in the chare Monitor
#endif
  Serial.println("Running");
}

float g[3];

void loop() {
  readAccelData(g);
  // See freescale AN3461
  float pitch = atan(g[0]/sqrt(pow(g[1],2) + pow(g[2],2)));
  float roll = atan(g[1]/sqrt(pow(g[0],2) + pow(g[2],2)));
  pitch *= 180.0/PI;
  roll *= 180.0/PI;

  Serial.print("p: ");
  Serial.print(pitch);
  Serial.print("  r: ");
  Serial.println(roll);

  int r_column = 2;
  int r_row = 0;

  if(!signbit(roll))
    r_column = 6;

  roll = fabs(roll);
  if(!signbit(g[2])){
    if (roll < 30)
      r_row = 0;
    else if (roll < 50)
      r_row = 1;
    else if (roll < 70)
      r_row = 2;
    else
      r_row = 3;     
  }
  else {
    if (roll < 30)
      r_row = 7;
    else if (roll < 50)
      r_row = 6;
    else if (roll < 70)
      r_row = 5;
    else
      r_row = 4;     
  }
  clearSphere();
  red[r_row][r_column] = 255;
  
  int p_column = 0;
  int p_row = 0;

  if(!signbit(pitch))
    p_column = 4;

  pitch = fabs(pitch);
  if(!signbit(g[2])){
    if (pitch < 30)
      p_row = 0;
    else if (pitch < 50)
      p_row = 1;
    else if (pitch < 70)
      p_row = 2;
    else
      p_row = 3;     
  }
  else {
    if (pitch < 30)
      p_row = 7;
    else if (pitch < 50)
      p_row = 6;
    else if (pitch < 70)
      p_row = 5;
    else
      p_row = 4;     
  }
  blue[p_row][p_column] = 255;
  
  
  
  //green[row][column] = 255;
  
  sendToSphere();
  delay(100);
}



void shake() {
  modeNum++;
  modeNum %= 8;
  shakeFlag = 1;
}

void modeChange() {
  exec = !exec;
}

#ifdef CHARGE_MONITOR
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
#endif /* CHARGE_MONITOR */

// ======================================================================================================

void sendToSphere(){
  // top strip
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 8; j++){
      topStrip.setPixelColor(indices[i][j], topStrip.Color(red[i][j], green[i][j], blue[i][j]));
    }
  }

  // bottom strip
  for(int i = 4; i < 8; i++){
    for(int j = 0; j < 8; j++){
      bottomStrip.setPixelColor(indices[i][j], bottomStrip.Color(red[i][j], green[i][j], blue[i][j]));
    }
  }
  topStrip.show();
  bottomStrip.show();
}

void clearSphere(){
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      red[i][j] = 0;
      blue[i][j] = 0;
      green[i][j] = 0;
    }
  } 
}








