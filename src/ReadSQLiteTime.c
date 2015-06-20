#include "bmp180d.h"

int ReadSQLiteTime(const char dbfile[200])
{
  sqlite3 *db;
  sqlite3_stmt *stmt; 
  const char query[200]="select datetime()";  
  int rc;
  int ok=0;

  char message[200]="";

  rc = sqlite3_open_v2(dbfile, &db, SQLITE_OPEN_READONLY, NULL);
  if( rc!=SQLITE_OK )
  {
    sprintf(message, "Can't open database: %s", sqlite3_errmsg(db));
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    sqlite3_close(db);
    return 0;
  }

  rc = sqlite3_prepare_v2(db, query, 200, &stmt, 0);
  if( rc==SQLITE_OK )
  {
    if( stmt!=NULL )
    {
      rc = sqlite3_step(stmt); 
      if( rc!=SQLITE_ROW )// could be SQLITE_BUSY here 
      {
        sprintf(message, "Statement failed: %s", sqlite3_errmsg(db));
        syslog(LOG_ERR|LOG_DAEMON, "%s", message);
        ok=0;
      }
      else
      {
        sprintf(message, "SQLite time: %s", sqlite3_column_text(stmt, 0));
        syslog(LOG_INFO|LOG_DAEMON, "%s", message);
        ok=1;
      }
    }
    else ok=0;
  }
  else
  {
    sprintf(message, "Statement prepare failed: %s", sqlite3_errmsg(db));
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    ok=0;
  }

  rc = sqlite3_finalize(stmt);
  sqlite3_close(db);

  return ok;
}

