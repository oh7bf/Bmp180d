#ifndef INSERTSQLITE_H_INCLUDED
#define INSERTSQLITE_H_INCLUDED
void InsertSQLite(
const char dbfile[SQLITEFILENAME_SIZE], // database file name
const char query[SQLITEQUERY_SIZE], // statement string
const char name[SENSORNAME_SIZE], // tag to identify data, e.g. sensor name
int n,                            // number of doubles to insert
double data[SQLITE_DOUBLES]);      // array of doubles
#endif

