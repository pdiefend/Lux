#include <Wire.h>
#include <Math.h>
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

#define MMA8452_ADDRESS 0x1C  // 0x1C if SDA0 low
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG  0x0E
#define WHO_AM_I         0x0D
#define CTRL_REG1        0x2A
#define CTRL_REG2        0x2B
#define CTRL_REG3        0x2C  // Wake Interrupts
#define CTRL_REG4        0x2D  // Interupt Enables
#define CTRL_REG5        0x2E  // Interrupt Config
#define ASLP_COUNT       0x29  // Auto-Sleep Counter
#define FF_MT_CFG        0x15  // Freefall Config
#define FF_MT_SRC        0x16  // Freefall Source
#define FF_MT_THS        0x17  // Freefall Threshold
#define FF_MT_COUNT      0x18  // Freefall Debounce Counter
#define TRANSIENT_CFG    0x1D  // Transient Config
#define TRANSIENT_SRC    0x1E  // Transient Source
#define TRANSIENT_THS    0x1F  // Transient Threshold
#define TRANSIENT_COUNT  0x20  // Transient Debounce Counter


#define GSCALE 2 // Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.
#define IMPACT 0x1F // sets the impact threshold to ~2g (31 * 0.0625g = 31/16)
#define FREEFALL 0x01 // sets the g-force threshold for freefall.

volatile byte shakeFlag = 0;
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
  //attachInterrupt(1, modeChange, FALLING);
  pinMode(ACCEL_INT, INPUT);
  //attachInterrupt(0, shake, FALLING);

  Wire.begin(); //Join the bus as a master
  Serial.println("Begin");

  initMMA8452(); //Test and intialize the MMA8452
#ifdef CHARGE_MONITOR
  initTimer(); // Start the Timer used in the chare Monitor
#endif
  Serial.println("Running");

  //red[0][7] = 255;
  //red[1][7] = 255;
  //red[2][7] = 255;
  //red[3][7] = 255;

  //blue[0][4] = 255;
  //blue[0][5] = 255;
  //blue[0][6] = 255;
  //blue[0][7] = 255;

  //green[0][4] = 255;
  //green[1][5] = 255;
  //green[2][6] = 255;
  //green[3][7] = 255;
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
  
  p_column = 0;
  p_row = 0;

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
  /*

   
   Serial.print("x: ");
   Serial.print(g[0]);
   Serial.print("  y: ");
   Serial.print(g[1]);
   Serial.print("  z: ");
   Serial.print(g[2]);
   
   Serial.print("    ");
   
   
   
   Serial.print("p: ");
   Serial.print(pitch);
   Serial.print("  r: ");
   Serial.println(roll);
   
   delay(500);
   */
}

// ======================================================================================================

void readAccelData(float * destination)
{
  byte rawData[6];  // x/y/z accel register data stored here

  readRegisters(OUT_X_MSB, 6, rawData);  // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for (int i = 0; i < 3 ; i++)
  {
    int gCount = (rawData[i * 2] << 8) | rawData[(i * 2) + 1]; //Combine the two 8 bit registers into one 12-bit number
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if (rawData[i * 2] > 0x7F)
    {
      gCount = ~gCount + 1;
      gCount *= -1;  // Transform into negative 2's complement #
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
    destination[i] /= 1000;
  }
}

// Initialize the MMA8452 registers
// See the many application notes for more info on setting all of these registers:
void initMMA8452()
{
  byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
  if (!(c == 0x2A)) // WHO_AM_I should always be 0x2A
  {
    Serial.print("Could not connect to MMA8452Q: 0x");
    Serial.println(c, HEX);
    while (1) ; // Loop forever if communication doesn't happen
  }

  MMA8452Standby();  // Must be in standby to change registers

  // Set up the full scale range to 2, 4, or 8g.
  byte fsr = GSCALE;
  if (fsr > 8) fsr = 8; //Easy error check
  fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, fsr);

  writeRegister(TRANSIENT_CFG, 0x1E);
  writeRegister(TRANSIENT_THS, IMPACT);
  writeRegister(TRANSIENT_COUNT, 0x01);

  writeRegister(FF_MT_CFG, 0xB8);
  writeRegister(FF_MT_THS, FREEFALL);
  writeRegister(FF_MT_COUNT, 0x08);

  writeRegister(CTRL_REG4, 0x24); // 0000 0100
  writeRegister(CTRL_REG5, 0x24); // 0000 0100

  MMA8452Active();  // Set to active to start reading
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}

// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

    Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); //Ask for bytes, once done, bus is released by default

  while (Wire.available() < bytesToRead); //Hang out until we get the # of bytes we expect

  for (int x = 0 ; x < bytesToRead ; x++)
    dest[x] = Wire.read();
}

// Read a single byte from addressToRead and return it as a byte
byte readRegister(byte addressToRead)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

    Wire.requestFrom(MMA8452_ADDRESS, 1); //Ask for 1 byte, once done, bus is released by default

  while (!Wire.available()) ; //Wait for the data to come back
  return Wire.read(); //Return this one byte
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission(); //Stop transmitting
}

// ======================================================================================================

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








