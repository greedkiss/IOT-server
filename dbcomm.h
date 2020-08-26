#ifndef ENVDATACOLLECTOR_DBCOMM_H
#define ENVDATACOLLECTOR_DBCOMM_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <mysql/mysql.h>
#include "devinfo.h"

#define MAX_SQL_SIZE    1024    //the maximum size of SQL (FOR ALL!)

MYSQL *g_conn;
MYSQL_RES *g_res;
MYSQL_ROW g_row;

//static char *g_host_name = "39.100.89.12";
//static char *g_host_name = "39.108.154.79";
static char *g_host_name = "120.79.79.43";
//static char *g_user_name = "root";
//static char *g_user_name = "wangxi";
static char *g_user_name = "database_songcongzhi";
//static char *g_password = "some_pass";
//static char *g_password = "XS3jBXzbNeF7U2CF";
static char *g_password = "mIRLcUVUAx0xfTGP";
//static char *g_db_name = "FIRST";
static char *g_db_name = "Sheep";
static unsigned int g_db_port = 3306;

/**
 * Print MySQL error message
 * @param msg
 */
void printMySQLError(const char *msg);

/**
 * Execute SQL
 * @param sql
 * @return 0 for succeed | -1 for failed
 */
int execSQL(const char *sql);

/**
 * Initialize MySQL
 * @return 0 for succeed | -1 for failed
 */
int initMySQL();

/**
 * Disconnect with MySQL
 * @return 0
 */
int closeMySQL();

/**
 * Generate a SQL with temperature, humidity or pH data
 * @param sql
 * @param data
 * @param sensorType
 * @param dataType
 * @param addr
 * @return 0 for succeed | -1 for wrong type
 */
int genTempOrHumOrPhSQL(char *sql, float data, int sensorType, int dataType, unsigned char addr);

/**
 * Generate a SQL with PM1, PM2.5, PM10 or NH3 data
 * @param sql
 * @param data
 * @param sensorType
 * @param dataType
 * @param addr
 * @return 0 for succeed | -1 for wrong type
 */
int genPMOrNH3SQL(char *sql, int data, int sensorType, int dataType, unsigned char addr);

#endif //ENVDATACOLLECTOR_DBCOMM_H
