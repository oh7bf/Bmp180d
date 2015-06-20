#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <sqlite3.h>
#include <errno.h>
#include <limits.h>


#ifndef BMP180D_H_INCLUDED
#define BMP180D_H_INCLUDED

#define I2LOCK_MAX 10
#define BMP180_ADDRESS 0x77
#define BMP180_AC1 0xAA
#define BMP180_AC2 0xAC
#define BMP180_AC3 0xAE
#define BMP180_AC4 0xB0
#define BMP180_AC5 0xB2
#define BMP180_AC6 0xB4
#define BMP180_B1 0xB6
#define BMP180_B2 0xB8
#define BMP180_MB 0xBA
#define BMP180_MC 0xBC
#define BMP180_MD 0xBE
#define BMP180_OUT 0xF6
#define BMP180_CTRL_MEAS 0xF4
#define BMP180_SOFT_RESET 0xE0
#define BMP180_ID 0xD0

#define SENSORNAME_SIZE 20
#define SQLITEFILENAME_SIZE 200
#define SQLITEQUERY_SIZE 200
#define SQLITE_DOUBLES 10 

extern const char *i2cdev;// i2c device
extern int loglev; // log level
extern int cont; // main loop flag

#endif

