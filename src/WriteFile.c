#include "bmp180d.h"

void WriteFile(const char fname[200], double value)
{
  char message[200]=""; 
  FILE *tfile;
  tfile=fopen(fname, "w");
  if(NULL==tfile)
  {
    sprintf(message,"could not write to file: %s",fname);
    syslog(LOG_ERR|LOG_DAEMON, message);
  }
  else
  { 
    fprintf(tfile,"%-6.3f",value);
    fclose(tfile);
  }
}


