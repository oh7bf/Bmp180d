#include "bmp180d.h"
// insert data to SQLite database table
void InsertSQLite(dbfile, query, name, n, data)
const char dbfile[SQLITEFILENAME_SIZE]; // database file name
const char query[SQLITEQUERY_SIZE]; // statement string
const char name[SENSORNAME_SIZE]; // tag to identify data, e.g. sensor name
int n;                            // number of doubles to insert
double data[SQLITE_DOUBLES];      // array of doubles
{
  sqlite3 *db;
  sqlite3_stmt *stmt; 
  char message[200]="";

  int rc;

  rc = sqlite3_open_v2(dbfile, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc!=SQLITE_OK ){
    sprintf(message, "Can't open database: %s", sqlite3_errmsg(db));
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    sqlite3_close(db);
    return;
  }

  int i;
  rc = sqlite3_prepare_v2(db, query, 200, &stmt, 0);
  if( rc==SQLITE_OK )
  {
    rc = sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
    if( rc!=SQLITE_OK)
    {
      sprintf(message, "Binding failed: %s", sqlite3_errmsg(db));
      syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    }

    for(i=1;i<=n;i++)
    {
       rc = sqlite3_bind_double(stmt, i+1, data[i-1]);
       if( rc!=SQLITE_OK)
       {
         sprintf(message, "Binding failed: %s", sqlite3_errmsg(db));
         syslog(LOG_ERR|LOG_DAEMON, "%s", message);
       }

    }

    rc = sqlite3_step(stmt); 
    if( rc!=SQLITE_DONE )// could be SQLITE_BUSY here 
    {
      sprintf(message, "Statement failed: %s", sqlite3_errmsg(db));
      syslog(LOG_ERR|LOG_DAEMON, "%s", message);
    }
  }
  else
  {
    sprintf(message, "Statement prepare failed: %s", sqlite3_errmsg(db));
    syslog(LOG_ERR|LOG_DAEMON, "%s", message);
  }

  rc = sqlite3_finalize(stmt);
  sqlite3_close(db);

  return;
}

