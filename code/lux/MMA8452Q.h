#include <Arduino.h>
#include <Wire.h>

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



void readAccelData(float * destination);
void initMMA8452();
void MMA8452Standby();
void MMA8452Active();
void readRegisters(byte addressToRead, int bytesToRead, byte * dest);
byte readRegister(byte addressToRead);
void writeRegister(byte addressToWrite, byte dataToWrite);

