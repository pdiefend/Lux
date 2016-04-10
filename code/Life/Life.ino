//#include <Wire.h>
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
#define PRELOAD 59286 // preload timer 65536-16MHz/256/10Hz

// Globals
Adafruit_NeoPixel topStrip = Adafruit_NeoPixel(nLEDs, TOP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bottomStrip = Adafruit_NeoPixel(nLEDs, BOTTOM, NEO_GRB + NEO_KHZ800);

const byte indices[8][8] = {
  {
    31,24,23,16,15,8,7,0              }
  ,{
    30,25,22,17,14,9,6,1              }
  ,{
    29,26,21,18,13,10,5,2              }
  ,{
    28,27,20,19,12,11,4,3              }
  ,{
    28,27,20,19,12,11,4,3              }
  ,{
    29,26,21,18,13,10,5,2              }
  ,{
    30,25,22,17,14,9,6,1              }
  ,{
    31,24,23,16,15,8,7,0              }
};

/*
// start with glyder
 byte red[8][8]= {
 {  0,  0,  0,  0,  0,  0,  0,  0},
 {  0,  0,  0,  0,  0,  0,  0,  0},
 {  0,  0,  0,  0,  0,  0,  0,  0},
 {  0,  0,  0,  0,  0,  0,  0,  0},
 {  0,  0,  0,  0,255,  0,255,  0},
 {  0,  0,  0,  0,  0,255,255,  0},
 {  0,  0,  0,  0,  0,255,  0,  0},
 {  0,  0,  0,  0,  0,  0,  0,  0},
 };
 */
byte red[8][8];
byte blue[8][8];
byte green[8][8];

boolean exec = false;

// ======================================================================================================

void setup() {
  //  long tick = millis();
  Serial.begin(115200);

  topStrip.begin();
  bottomStrip.begin();
  topStrip.show();
  bottomStrip.show();
  topStrip.setBrightness(BRIGHTNESS);
  bottomStrip.setBrightness(BRIGHTNESS);

  pinMode(MODE_INT, INPUT);

  Serial.println("Begin");

  //clearSphere(1);


  randomSeed(analogRead(0));
  long randNum = 0;
  // init game with random pattern
  for(int idx = 0; idx<8; idx++){
    for(int jdx = 0; jdx<8; jdx++){
      randNum = random(5);
      if(randNum == 0){
        red[idx][jdx]=255;
      }
    }
  }
}

void loop() {

  doGameOfLifeTick(0);

  sendToSphere();
  delay(100);
}

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

void clearSphere(int sendData){
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      red[i][j] = 0;
      blue[i][j] = 0;
      green[i][j] = 0;
    }
  }
  if(sendData){
    sendToSphere();
  } 
}

// Game of life specific functions:

void doGameOfLifeTick(int gameMode){
  int neighbors = 0;
  byte nextRed[8][8];

  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      nextRed[i][j] = 0;
    }
  }

  for(int idx = 0; idx<8; idx++){
    for(int jdx = 0; jdx<8; jdx++){
      long rand = random(10);
      // first count up all the neighbors for each cell       
      neighbors = countNeighbors(idx, jdx);

      switch (gameMode){
      case 0: // play using traditional rules
        {
          /*
           * Traditional Rules
           * Any live cell with fewer than two live neighbours dies, as if caused by under-population.
           * Any live cell with two or three live neighbours lives on to the next generation.
           * Any live cell with more than three live neighbours dies, as if by over-population.
           * Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
           */
          // first see if the cell is alive
          if(red[idx][jdx] != 0){ // if alive
            if(neighbors < 2){
              nextRed[idx][jdx] = 0;
            } 
            else if (neighbors > 3) {
              nextRed[idx][jdx] = 0;
            } 
            else {
              nextRed[idx][jdx] = 255;
            }
          } 
          else { // if dead
            if(neighbors == 3){
              nextRed[idx][jdx] = 255; 
            }
          }
          break;
        }
      case 1: // Probabilistically Enhanced (lasts longer)
        {
          // Probabilistic-based rules
          // first see if the cell is alive
          if(red[idx][jdx] != 0){ // if alive
            if((neighbors < 2) && (rand > 2)){ // if too few neighbors and 20%
              nextRed[idx][jdx] = 0;
            } 
            else if ((neighbors > 3) && (rand > 2)) { // if too many neighbors and 20%
              nextRed[idx][jdx] = 0;
            } 
            else { // else we have the right amount to survive
              if(rand < 5) { // with 80% chance to live
                nextRed[idx][jdx] = 255;
              } 
              else {
                nextRed[idx][jdx] = 0;
              }
            }
          } 
          else { // if dead
            if((neighbors == 3) && (rand > 4)){ // can come alive if correct number of neighbors
              nextRed[idx][jdx] = 255; 
            } 
            else { // small chance to come alive spontaneously 10%
              if(rand == 9){
                nextRed[idx][jdx] = 255; 
              } 
            }
          }
          break;
        }
      }
    } 
  }

  for(int idx = 0; idx<8; idx++){
    for(int jdx = 0; jdx<8; jdx++){
      red[idx][jdx] = nextRed[idx][jdx];     
    }
  }  
}

byte countNeighbors(int idx, int jdx){
  byte neighbors = 0;

  byte left = 0;
  byte right = 0;
  byte up = 0;
  byte down = 0;

  // get index of left neighbors
  if(idx == 0){
    left = 7;
  } 
  else {
    left = idx-1;
  }    

  // get index of right neighbors
  if(idx == 7){
    right = 0;
  } 
  else {
    right = idx+1;
  }    

  // get index of upper neighbors
  if(jdx == 0){
    up = 7;
  } 
  else {
    up = jdx-1;
  }    

  // get index of lower neighbors
  if(jdx == 7){
    down = 0;
  } 
  else {
    down = jdx+1;
  }

  if(red[left][jdx] != 0){
    neighbors++;
  }
  if(red[left][up] != 0){
    neighbors++;
  }
  if(red[left][down] != 0){
    neighbors++;
  }
  if(red[right][jdx] != 0){
    neighbors++;
  }
  if(red[right][up] != 0){
    neighbors++;
  }
  if(red[right][down] != 0){
    neighbors++;
  }
  if(red[idx][up] != 0){
    neighbors++;
  }
  if(red[idx][down] != 0){
    neighbors++;
  }
  return neighbors; 
}




