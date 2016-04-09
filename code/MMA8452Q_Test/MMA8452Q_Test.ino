#include <Wire.h>
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
#define IMPACT 0x1F // sets the impact threshold to 4g (64 * 0.0625g = 64/16)
#define FREEFALL 0x01 // sets the g-force threshold for freefall.

volatile byte shakeFlag = 0;
volatile byte modeNum = 0;

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
  Serial.println("Running");
}

void loop() {
  if (exec)
    Serial.print("executing");
  else
    Serial.print("not executing");
  Serial.print(" ");
  Serial.println(modeNum);

  if(shakeFlag){
    readRegister(FF_MT_SRC); // clear the interrupt
        readRegister(TRANSIENT_SRC); // clear the interrupt
  }
  
  if (exec) {
    switch (modeNum) {
      case 1:
        colorWipe(topStrip.Color(255, 0, 0), 50);
        break;
      case 2:
        colorWipe(topStrip.Color(255, 255, 0), 50);
        break;
      case 3:
        colorWipe(topStrip.Color(0, 255, 0), 50);
        break;
      case 4:
        colorWipe(topStrip.Color(0, 255, 255), 50);
        break;
      case 5:
        colorWipe(topStrip.Color(0, 0, 255), 50);
        break;
      case 6:
        colorWipe(topStrip.Color(255, 0, 255), 50);
        break;
      case 7:
        colorWipe(topStrip.Color(255, 255, 255), 50);
        break;
      default:
        colorWipe(topStrip.Color(0, 0, 0), 50);
        break;
    }
  } else
    colorWipe(topStrip.Color(0, 0, 0), 1);

//  digitalWrite(LED, HIGH);
//  delay(100);
//  digitalWrite(LED, LOW);
//  delay(100);
//
//  if (digitalRead(BATT_STAT) == HIGH) { // if stat is high, charge complete
//    digitalWrite(LED, HIGH);
//    delay(100);
//    digitalWrite(LED, LOW);
//  }
//  delay(500);
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


void readAccelData(int * destination)
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
  }
}

// Initialize the MMA8452 registers
// See the many application notes for more info on setting all of these registers:
void initMMA8452()
{
  byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
  if (c == 0x2A) // WHO_AM_I should always be 0x2A
  {
    Serial.println("MMA8452Q is online...");
  }
  else
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

void shake() {
  modeNum++;
  modeNum %= 8;
  shakeFlag = 1;
}

void modeChange() {
  exec = !exec;
}
