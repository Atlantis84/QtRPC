#ifndef DMDBACCESS_GLOBAL_H
#define DMDBACCESS_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#if defined(DMDBACCESS_LIBRARY)
#  define DMDBACCESS_EXPORT Q_DECL_EXPORT
#else
#  define DMDBACCESS_EXPORT Q_DECL_IMPORT
#endif

//1、Info for DBAccess;
//2、for test,need to change for real use;
//3、through ODBC to conn DM Database on the vitural machine
#define  HOSTNAME           "127.0.0.1"
#define  DATABASENAME       "TestBackup"
#define  USERNAME           "postgres"
#define  PASSWORD           "a"
#define  PORTNUM             5432

#define  HOSTNAME_MYSQL           "127.0.0.1"
#define  DATABASENAME_MYSQL       "tv4th"
#define  USERNAME_MYSQL           "root"
#define  PASSWORD_MYSQL          "asdfasdf1"
#define  PORTNUM_MYSQL             3306

//style QODBC/QODBC3/QSQLITE/QPSQL/QPSQL7
#define  DATABASESTYLE       "QPSQL"
#define  DATABASESTYLE_MYSQL       "QMYSQL"

//data table defination
const QString constUserInfoTable        = QString("\"User_Table\"");         //用户数据信息表
const QString constCalibrationEntryTable= QString("\"Calibration_Entry_Table\"");
const QString constCalibrationDetailsTable = QString("\"Calibration_Details_Table\"");
const QString constRegisterMachineTable = QString("\"Machine_Register_Table\"");
const QString constCurrentChangeInfoTable = QString("\"Current_Product_Station_Info\"");
#endif // DMDBACCESS_GLOBAL_H
