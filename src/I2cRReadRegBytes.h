#ifndef I2CRREADREGBYTES_H_INCLUDED
#define I2CRREADREGBYTES_H_INCLUDED
int I2cRReadRegBytes(
int address, // i2c address 
unsigned char reg, // register
unsigned char *buffer, // buffer to fill
int nbytes // number of bytes to read
);
#endif
