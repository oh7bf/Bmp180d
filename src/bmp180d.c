/**************************************************************************
 * 
 * Read pressure from BMP180 chip with I2C and write it to a file and SQLite
 * database. 
 *       
 * Copyright (C) 2015 Jaakko Koivuniemi.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************
 *
 * Fri Jun 12 22:33:22 CEST 2015
 * Edit: Sat Jun 20 18:58:01 CEST 2015
 *
 * Jaakko Koivuniemi
 **/

#include "bmp180d.h"
#include "I2cWriteRegister.h"
#include "I2cRReadRegBytes.h"
#include "WriteFile.h"
#include "ReadSQLiteTime.h"
#include "InsertSQLite.h"

const int version=20150620; // program version
int presint=300; // pressure measurement interval [s]

short AC1, AC2, AC3, B1, B2, MB, MC, MD;
unsigned short AC4, AC5, AC6;

const char adatafile[200]="/var/lib/bmp180d/altitude";
const char pdatafile[200]="/var/lib/bmp180d/pressure";
const char tdatafile[200]="/var/lib/bmp180d/temperature";

// local SQLite database file
int dbsqlite=0; // store data to local SQLite database
char dbfile[ SQLITEFILENAME_SIZE ];
const char query[ SQLITEQUERY_SIZE ] = "insert into bmp180 (name,temperature,pressure,altitude) values (?,?,?,?)";

const char *i2cdev = "/dev/i2c-1";

const char confile[200]="/etc/bmp180d_config";

const char pidfile[200]="/var/run/bmp180d.pid";

int loglev=5;
const char logfile[200]="/var/log/bmp180d.log";
char message[200]="";

void read_config()
{
  FILE *cfile;
  char *line=NULL;
  char par[20];
  float value;
  size_t len;
  ssize_t read;

  cfile=fopen(confile, "r");
  if(NULL!=cfile)
  {
    syslog(LOG_INFO|LOG_DAEMON, "Read configuration file");
    while((read=getline(&line,&len,cfile))!=-1)
    {
       if(sscanf(line,"%s %f",par,&value)!=EOF) 
       {
          if(strncmp(par,"LOGLEVEL",8)==0)
          {
             loglev=(int)value;
             sprintf(message,"Log level set to %d",(int)value);
             syslog(LOG_INFO|LOG_DAEMON, "%s", message);
             setlogmask(LOG_UPTO (loglev));
          }
          if(strncmp(par,"DBSQLITE",8)==0)
          {
            if(sscanf(line,"%s %s",par,dbfile)!=EOF)  
            {
              dbsqlite=1;
              sprintf(message, "Store data to database %s", dbfile);
              syslog(LOG_INFO|LOG_DAEMON, "%s", message);
            }
          }
          if(strncmp(par,"PRESINT",7)==0)
          {
             presint=(int)value;
             sprintf(message,"Pressure measurement interval set to %d s",(int)value);
             syslog(LOG_INFO|LOG_DAEMON, "%s", message);
          }
       }
    }
    fclose(cfile);
  }
  else
  {
    sprintf(message, "Could not open %s", confile);
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
  }
}

int cont=1; /* main loop flag */

void BMP180ReadCalibration()
{
  unsigned char buffer[20];

  if( I2cRReadRegBytes(BMP180_ADDRESS, BMP180_AC1, buffer, 22) == 1)
  {
    AC1 = ((short)buffer[0])<<8;
    AC1 |= (short)buffer[1];
    sprintf(message, "AC1 = %d", AC1);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    AC2 = ((short)buffer[2])<<8;
    AC2 |= (short)buffer[3];
    sprintf(message, "AC2 = %d", AC2);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    AC3 = ((short)buffer[4])<<8;
    AC3 |= (short)buffer[5];
    sprintf(message, "AC3 = %d", AC3);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    AC4 = ((unsigned short)buffer[6])<<8;
    AC4 |= (unsigned short)buffer[7];
    sprintf(message, "AC4 = %d", AC4);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    AC5 = ((unsigned short)buffer[8])<<8;
    AC5 |= (unsigned short)buffer[9];
    sprintf(message, "AC5 = %d", AC5);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    AC6 = ((unsigned short)buffer[10])<<8;
    AC6 |= (unsigned short)buffer[11];
    sprintf(message, "AC6 = %d", AC6);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    B1 = ((short)buffer[12])<<8;
    B1 |= (short)buffer[13];
    sprintf(message, "B1 = %d", B1);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    B2 = ((short)buffer[14])<<8;
    B2 |= (short)buffer[15];
    sprintf(message, "B2 = %d", B2);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    MB = ((short)buffer[16])<<8;
    MB |= (short)buffer[17];
    sprintf(message, "MB = %d", MB);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    MC = ((short)buffer[18])<<8;
    MC |= (short)buffer[19];
    sprintf(message, "MC = %d", MC);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

    MD = ((short)buffer[20])<<8;
    MD |= (short)buffer[21];
    sprintf(message, "MD = %d", MD);
    syslog(LOG_NOTICE|LOG_DAEMON, message);

  }
  else
  {
    syslog(LOG_ERR|LOG_DAEMON, "failed to read AC1...MD");
    cont = 0;
  }
}

void Bmp180ReadTempPress()
{
  long UT, UP, X1, X2, X3, B3, B5, B6, p;
  unsigned long B4, B7;
  double T = -100;
  double data[ SQLITE_DOUBLES ];
  unsigned char buffer[10];

  if( I2cWriteRegister(BMP180_ADDRESS, BMP180_CTRL_MEAS, 0x2E) == 1 )
  {
    usleep(4500);
    if( I2cRReadRegBytes(BMP180_ADDRESS, BMP180_OUT, buffer, 2) == 1 )
    {
      UT = ((long)buffer[0])<<8;
      UT |= (long)buffer[1]; 
      X1 = ((UT-AC6)*AC5)>>15;
      X2 = (((long)MC)<<11)/(X1+MD);
      B5 = X1 + X2;
      T = 0.1*((double)((B5+8)>>4));

      if( I2cWriteRegister(BMP180_ADDRESS, BMP180_CTRL_MEAS, 0xF4) == 1 )
      {
        usleep(25500);
        if( I2cRReadRegBytes(BMP180_ADDRESS, BMP180_OUT, buffer, 3) == 1 )
        {
          UP = ((long)buffer[0])<<16;
          UP |= ((long)buffer[1])<<8;
          UP |= ((long)buffer[2]);
          UP = UP>>5;
          B6 = B5 - 4000;
          X1 = (B2*((B6*B6)>>12))>>11;
          X2 = (AC2*B6)>>11;
          X3 = X1+X2;
          B3 = (((AC1*4+X3)<<3)+2)>>2;
          X1 = (AC3*B6)>>13;
          X2 = (B1*((B6*B6)>>12))>>16;
          X3 = ((X1+X2)+2)>>2;
          B4 = (AC4*((unsigned long)(X3+32768)))>>15;
          B7 = ((unsigned long)UP-B3)*(50000>>3);
          if( B7 < 0x80000000 ) p = (B7*2)/B4;
          else p = B7/B4*2;
          X1 = (p>>8)*(p>>8);
          X1 = (X1*3038)>>16;
          X2 = (-7357*p)>>16;
          p = p+(X1+X2+3791)/16;

          sprintf(message, "T=%+6.2f C p=%ld Pa", T, p);
          syslog(LOG_INFO|LOG_DAEMON, message);

          WriteFile(tdatafile, T);
          WriteFile(pdatafile, p);
//          WriteFile(adatafile, a);

          data[0] = T;
          data[1] = p;
          data[2] = 0;

          if(dbsqlite==1) InsertSQLite(dbfile, query, "p1", 3, data);

        }
      }
    }
  }
}


void stop(int sig)
{
  sprintf(message, "signal %d catched, stop", sig);
  syslog(LOG_NOTICE|LOG_DAEMON, "%s", message);
  cont = 0;
}

void terminate(int sig)
{
  sprintf(message, "signal %d catched", sig);
  syslog(LOG_NOTICE|LOG_DAEMON, "%s", message);

  sleep(1);
  syslog(LOG_NOTICE|LOG_DAEMON, "stop");

  cont = 0;
}

void hup(int sig)
{
  sprintf(message, "signal %d catched", sig);
  syslog(LOG_NOTICE|LOG_DAEMON, "%s", message);
}


int main()
{  
  int ok=0;

  setlogmask(LOG_UPTO (loglev));
  syslog(LOG_NOTICE|LOG_DAEMON, "bmp180d v. %d started", version); 

  signal(SIGINT, &stop); 
  signal(SIGKILL, &stop); 
  signal(SIGTERM, &terminate); 
  signal(SIGQUIT, &stop); 
  signal(SIGHUP, &hup); 

  read_config();

  int unxs = (int)time(NULL); // unix seconds
  int nxtpres = unxs; // next time to read pressure and temperature

  pid_t pid, sid;
        
  pid=fork();
  if(pid<0) 
  {
    exit(EXIT_FAILURE);
  }

  if(pid>0) 
  {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if( sid < 0 ) 
  {
    syslog(LOG_ERR|LOG_DAEMON, "failed to create child process"); 
    exit(EXIT_FAILURE);
  }
        
  if((chdir("/")) < 0) 
  {
    syslog(LOG_ERR|LOG_DAEMON, "failed to change to root directory"); 
    exit(EXIT_FAILURE);
  }
 
  /* Close out the standard file descriptors */
  close( STDIN_FILENO );
  close( STDOUT_FILENO );
  close( STDERR_FILENO );

  FILE *pidf;
  pidf = fopen(pidfile, "w");

  if( pidf == NULL )
  {
    sprintf(message,"Could not open PID lock file %s, exiting", pidfile);
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    exit(EXIT_FAILURE);
  }

  if( flock(fileno(pidf), LOCK_EX||LOCK_NB) == -1 )
  {
    sprintf(message, "Could not lock PID lock file %s, exiting", pidfile);
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    exit(EXIT_FAILURE);
  }

  fprintf(pidf, "%d\n", getpid());
  fclose(pidf);

  if( dbsqlite == 1 )
  {
    if( ReadSQLiteTime( dbfile ) == 0 ) 
    {
      syslog(LOG_ERR|LOG_DAEMON, "SQLite database read failed, drop database connection");
      dbsqlite=0; 
    }
  }

  unsigned char buffer[10];
  if( I2cRReadRegBytes(BMP180_ADDRESS, BMP180_ID, buffer, 1) == 1)
  {
    if( buffer[0] != 0x55 )
    {
      sprintf(message, "Chip ID 0x%02x != 0x55, exit", buffer[0]);
      syslog(LOG_ERR|LOG_DAEMON, message);
      cont = 0;
    }
    else
    {
      sprintf(message, "Chip ID 0x%02x", buffer[0]);
      syslog(LOG_INFO|LOG_DAEMON, message);

      BMP180ReadCalibration(); 
    }
  }
  else
  {
    syslog(LOG_ERR|LOG_DAEMON, "failed to read ID, exit");
    cont = 0;
  }

  while( cont == 1 )
  {
    unxs = (int)time(NULL); 

    if((( unxs >= nxtpres )||( (nxtpres-unxs) > presint ))&&( presint > 10 )) 
    {
      nxtpres = presint + unxs;

      Bmp180ReadTempPress();
    }

    sleep(1);
  }

  syslog(LOG_NOTICE|LOG_DAEMON, "remove PID file");
  ok = remove( pidfile );

  return ok;
}
