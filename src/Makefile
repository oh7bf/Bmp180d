# -*- mode: makefile -*-
# 
# Makefile for compiling 'bmp180d' on Raspberry Pi. 
#
# Wed Jun 10 21:51:27 CEST 2015
# Edit: 
# 
# 
# Jaakko Koivuniemi

CXX           = gcc
CXXFLAGS      = -g -O -Wall 
LD            = gcc
LDFLAGS       = -lm -lsqlite3 -O

%.o : %.c
	$(CXX) $(CXXFLAGS) -c $<

all: bmp180d 

bmp180d: I2cWriteRegister.o I2cRReadRegBytes.o WriteFile.o ReadSQLiteTime.o InsertSQLite.o bmp180d.o
	$(LD) $(LDFLAGS) $^ -o $@

test: test.o
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o

