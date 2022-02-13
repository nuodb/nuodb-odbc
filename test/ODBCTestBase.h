/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#define DBNAME          "ODBCTest"
#define DBHOST          "localhost:48004"
#define DBAUSER         "dba"
#define DBAPASSWORD     "dba"
#define SCHEMANAME      "ODBCTEST41"

#if defined(_WIN32)
# define ODBC_WINDOWS
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# define _CRT_SECURE_NO_WARNINGS
# include <windows.h>
# ifndef CP_UTF8
#  define CP_UTF8 65001
# endif
#endif

#include <sql.h>
#include <sqlext.h>

#if defined(ODBC_WINDOWS)
#include <odbcss.h>
#endif

#if !defined(ODBC_WINDOWS)
#include <odbcinstext.h>
#include "../src/SetupAttributes.h"
#endif

#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <math.h>

// A "pretty big" buffer
#define PRETTY_BIG 1024

// Since unix odbc doesn't support WCHAR use some indirection so we
// can get _some_ unix testing.
#if defined(ODBC_WINDOWS)
typedef SQLWCHAR TEST_CHAR_TYPE;
#define TEST_SQL_C_CHAR SQL_C_WCHAR
#else
typedef SQLCHAR TEST_CHAR_TYPE;
#define TEST_SQL_C_CHAR SQL_C_CHAR
#endif

// SUCCEED and FAIL are defined in obbcss.h as well, so on windows these need
// to be undefined or it generated warnings

#ifdef SUCCEED
# undef SUCCEED
#endif
#ifdef FAIL
# undef FAIL
#endif

#ifndef MIN
# define MIN(a, b) ((a < b) ? a : b)
#endif

#include "gtest/gtest.h"

#define NUMROWS         10
#define FIRSTSTRING     "BIGFIRSTNUMBER"
#define SECONDSTRING    "NOTSOBIGSECONDNUMBER"
#define BINARYSTRING    "BIGBINARYNUMBERISHERE"
#define CLOBSTRING      "BIGCLOBSTRINGISHERE"
#define VALSIZE         128
#define YEARBASE(index) (index % 100 + 1970)
#define NULLROW(index)  (index % 13 == 0)

// use template for char type so we can easily test both ways in the future
template<typename CHAR_TYPE>
class RowWiseBinding
{
public:
    RowWiseBinding(int numRowsPerFetch)
        : mxnameInd(numRowsPerFetch)
        , mxname(numRowsPerFetch*VALSIZE)
        , mxIntInd(numRowsPerFetch)
        , mxInt(numRowsPerFetch)
        , mxShortInd(numRowsPerFetch)
        , mxShort(numRowsPerFetch)
        , mxBigIntInd(numRowsPerFetch)
        , mxBigInt(numRowsPerFetch)
        , mxDoubleInd(numRowsPerFetch)
        , mxDouble(numRowsPerFetch)
        , mxTinyIntInd(numRowsPerFetch)
        , mxTinyInt(numRowsPerFetch)
        , mxFloatInd(numRowsPerFetch)
        , mxFloat(numRowsPerFetch)
        , mxDecimalInd(numRowsPerFetch)
        , mxDecimal(numRowsPerFetch*10)
        , mxtablespcInd(numRowsPerFetch)
        , mxtablespc(numRowsPerFetch*VALSIZE)
        , mxDateInd(numRowsPerFetch)
        , mxDate(numRowsPerFetch)
        , mxTimeInd(numRowsPerFetch)
        , mxTime(numRowsPerFetch)
        , mxTimeStampInd(numRowsPerFetch)
        , mxTimeStamp(numRowsPerFetch)
        , mxBinaryInd(numRowsPerFetch)
        , mxBinary(numRowsPerFetch*VALSIZE)
        , mxClobInd(numRowsPerFetch)
        , mxClob(numRowsPerFetch*VALSIZE)
        , rowStatus(numRowsPerFetch)
    {
    }

    std::vector<SQLLEN>               mxnameInd;
    std::vector<CHAR_TYPE>            mxname;
    std::vector<SQLLEN>               mxIntInd;
    std::vector<SQLINTEGER>           mxInt;
    std::vector<SQLLEN>               mxShortInd;
    std::vector<SQLSMALLINT>          mxShort;
    std::vector<SQLLEN>               mxBigIntInd;
    std::vector<SQLBIGINT>            mxBigInt;
    std::vector<SQLLEN>               mxDoubleInd;
    std::vector<SQLDOUBLE>            mxDouble;
    std::vector<SQLLEN>               mxTinyIntInd;
    std::vector<char>                 mxTinyInt;
    std::vector<SQLLEN>               mxFloatInd;
    std::vector<SQLREAL>              mxFloat;
    std::vector<SQLLEN>               mxDecimalInd;
    std::vector<CHAR_TYPE>            mxDecimal;
    std::vector<SQLLEN>               mxtablespcInd;
    std::vector<CHAR_TYPE>            mxtablespc;
    std::vector<SQLLEN>               mxDateInd;
    std::vector<SQL_DATE_STRUCT>      mxDate;
    std::vector<SQLLEN>               mxTimeInd;
    std::vector<SQL_TIME_STRUCT>      mxTime;
    std::vector<SQLLEN>               mxTimeStampInd;
    std::vector<SQL_TIMESTAMP_STRUCT> mxTimeStamp;
    std::vector<SQLLEN>               mxBinaryInd;
    std::vector<char>                 mxBinary;
    std::vector<SQLLEN>               mxClobInd;
    std::vector<CHAR_TYPE>            mxClob;
    std::vector<SQLUSMALLINT>         rowStatus;
};

template<typename CHAR_TYPE>
class ColumnBinding
{
public:
    CHAR_TYPE   mxname[VALSIZE];    // 1
    SQLLEN      mxnameInd;
    SQLINTEGER mxInt;   // 2
    SQLLEN mxIntInd;
    SQLSMALLINT mxShort;    // 3
    SQLLEN mxShortInd;
    SQLBIGINT   mxBigInt;   // 4
    SQLLEN      mxBigIntInd;
    SQLDOUBLE   mxDouble;   // 5
    SQLLEN      mxDoubleInd;
    SQLCHAR     mxTinyInt;  // 6
    SQLLEN      mxTinyIntInd;
    SQLREAL     mxFloat;    // 7
    SQLLEN      mxFloatInd;
    CHAR_TYPE   mxDecimal[VALSIZE]; // 8
    SQLLEN      mxDecimalInd;
    CHAR_TYPE   mxtablespc[VALSIZE]; // 9
    SQLLEN      mxtablespcInd;
    SQL_DATE_STRUCT mxDate; // 10
    SQLLEN mxDateInd;
    SQL_TIME_STRUCT mxTime; // 11
    SQLLEN mxTimeInd;
    SQL_TIMESTAMP_STRUCT mxTimeStamp; // 12
    SQLLEN  mxTimeStampInd;
    char    mxBinary[4099];  // 13 -- has to be big enough for DB915 test
    SQLLEN  mxBinaryInd;
    CHAR_TYPE   mxClob[VALSIZE];    // 14
    SQLLEN      mxClobInd;
};

template<typename CHAR_TYPE>
class ColumnWiseBinding
{
public:
    ColumnWiseBinding(int numRowsPerFetch)
    {
        rows = new ColumnBinding<CHAR_TYPE>[numRowsPerFetch];
        memset(rows, 0, sizeof(ColumnBinding<CHAR_TYPE>) * numRowsPerFetch);
        rowStatus = new SQLUSMALLINT[numRowsPerFetch];
    }
    ~ColumnWiseBinding()
    {
        delete [] rows;
        delete [] rowStatus;
    }
    ColumnBinding<CHAR_TYPE>*  rows;
    SQLUSMALLINT*   rowStatus;
};

class ColAttributes {

public:
    ColAttributes() {
        //
    }

    std::set<unsigned int> valid;
    bool present(unsigned int item) { return valid.count(item) > 0; }
    bool autoUniqueValue;
    std::string baseColumnName;
    std::string baseTableName;
    std::string caseSensitive;
    std::string catalogName;
    SQLLEN conciseType;
    SQLLEN count; // really not for a single col.
    SQLLEN displaySize;
    bool fixedPrecScale;
    std::string label;
    SQLLEN length;
    std::string literalPrefix;
    std::string literalSuffix;
    std::string localTypeName;
    std::string name;
    SQLLEN nullable;
    SQLLEN numPrecRadix;
    SQLLEN octetLength;
    SQLLEN precision;
    SQLLEN scale;
    std::string schemaName;
    SQLLEN searchable;
    std::string tableName;
    SQLLEN type;
    std::string typeName;
    SQLLEN unnamed;
    bool unsigned_;
    SQLLEN updatable;
};


class ODBCTestBase : public testing::Test
{
public:
    ODBCTestBase()
    {
        henv = NULL;
        hdbc1 = NULL;
        stmt = NULL;
    }


#if !defined(ODBC_WINDOWS)
    /* With unixODBC we can 'install' the driver locally.
       On Windows, that requires admin privileges.

       ODBCINSTINI=odbcinst.ini
       ODBCSYSINI=/tmp/
    */

    static void SetUpTestCase()
    {
        // Set up the environment for the unixODBC driver
        const char* tmp = getenv("NUOODBC_TEST_TEMP");
        setenv("ODBCSYSINI", tmp ? tmp : "/tmp", 0);
        setenv("ODBCINSTINI", "odbcinst.ini", 0);
    }
#endif

    void SetUp() override
    {
        connect();
    }

    void TearDown() override
    {
        disconnect();
    }

    void disconnect()
    {
        RETCODE retcode;

        if (stmt) {
            retcode =   SQLFreeStmt(stmt, SQL_CLOSE);
            ASSERT_TRUE(hdbc1 == NULL ? retcode != SQL_SUCCESS : retcode == SQL_SUCCESS);

            retcode =   SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            ASSERT_TRUE(hdbc1 == NULL ? retcode != SQL_SUCCESS : retcode == SQL_SUCCESS);

            stmt = NULL;
        }

        if (hdbc1) {
            retcode = SQLDisconnect(hdbc1);
            ASSERT_TRUE(hdbc1 == NULL ? retcode != SQL_SUCCESS : retcode == SQL_SUCCESS);

            retcode = SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
            ASSERT_TRUE(hdbc1 == NULL ? retcode != SQL_SUCCESS : retcode == SQL_SUCCESS);

            hdbc1 = NULL;
        }

        if (henv) {
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            henv = NULL;
        }
    }

    void connect()
    {
        henv = NULL;
        hdbc1 = NULL;
        stmt = NULL;

        // Allocate the ODBC environment and save handle.
        RETCODE retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
        ASSERT_TRUE(retcode == SQL_SUCCESS) << "Failed to allocate handle";

        // Notify ODBC that this is an ODBC 3.0 app.
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
                                (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
        ASSERT_TRUE(retcode == SQL_SUCCESS) << "Failed to set version 3.0";

        newConnection(hdbc1);

        // set AUTOCOMMIT on
        retcode = SQLSetConnectAttr(hdbc1,
                                    SQL_ATTR_AUTOCOMMIT,
                                    (SQLPOINTER)SQL_AUTOCOMMIT_ON,
                                    SQL_NTS);
        ASSERT_TRUE(retcode == SQL_SUCCESS);

        // Allocate statement handle.
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &stmt);
        ASSERT_TRUE(retcode == SQL_SUCCESS);

        execDirect("drop schema " SCHEMANAME " cascade");
        execDirect("use " SCHEMANAME);
    }

#if defined(ODBC_WINDOWS)
    const std::string convertToUTF8FromUnicode(SQLWCHAR* src)
    {
        char dest[PRETTY_BIG*10];

        if (src) {
            size_t len = wcslen(src);

            if (len > 0) {
                int numchars = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
                numchars = WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, numchars, NULL, NULL);
            } else {
                *dest = '\0';
            }
        } else {
            *dest = '\0';
        }

        return std::string(dest);
    }
#else
    // We only W stuff on windows.
    const std::string convertToUTF8FromUnicode(SQLCHAR* src)
    {
        return std::string((const char*)src);
    }
#endif
    void newConnection(HDBC& dbc) {
        dbc = NULL;
        ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_DBC, henv, &dbc));

#if !defined(ODBC_WINDOWS)
        char libPath[PRETTY_BIG];

        const char* pth = getenv("NUOODBC_LIB");
        if (pth == nullptr || *pth == '\0') {
            fprintf(stderr, "NUOODBC_LIB is not set\n");
            exit(EXIT_FAILURE);
        }
        strcpy(libPath, pth);

        if (access(libPath, R_OK) != 0) {
            fprintf(stderr, "NUOODBC_LIB %s does not exist\n", libPath);
            exit(EXIT_FAILURE);
        }

        std::vector<char> params = setupDriverKeys(libPath);

        char pathOut[PRETTY_BIG];
        WORD pathOutLen;
        DWORD usage;
        char const* paramsChars = &params[0];
        if (!SQLInstallDriverEx(paramsChars,
                                NULL,
                                pathOut,
                                PRETTY_BIG,
                                &pathOutLen,
                                ODBC_INSTALL_INQUIRY,
                               &usage)) {
            reportInstallerErrors();
            FAIL();
        }
        if (!SQLInstallDriverEx(paramsChars,
                                NULL,
                                libPath,
                                PRETTY_BIG,
                                &pathOutLen,
                                ODBC_INSTALL_COMPLETE,
                                &usage)) {
            reportInstallerErrors();
            FAIL();
        }
#endif
        const char* dbname = getenv("NUODB_DBNAME");
        if (!dbname) {
            dbname = DBNAME;
        }

        const char* dbhost = getenv("NUODB_HOST");
        if (!dbhost) {
            dbhost = "localhost:48004";
        }

        const char* dbauser = getenv("NUODB_USER");
        if (!dbauser) {
            dbauser = DBAUSER;
        }
        const char* dbapwd = getenv("NUODB_PASSWORD");
        if (!dbapwd) {
            dbapwd = DBAPASSWORD;
        }

        char connectString[PRETTY_BIG];
        sprintf(connectString, "Driver=NuoDB ODBC Driver;UID=%s;PWD=%s;DBNAME=%s@%s;",
                dbauser, dbapwd, dbname, dbhost);

        char outConnectString[PRETTY_BIG];
        SQLSMALLINT strlen2 = 0;

        RETCODE ret = SQLDriverConnect(dbc,
                                       NULL,
                                       (SQLCHAR*)connectString,
                                       (SQLSMALLINT)strlen(connectString),
                                       (SQLCHAR*)outConnectString,
                                       PRETTY_BIG-1,
                                       &strlen2,
                                       SQL_DRIVER_NOPROMPT);

        outConnectString[strlen2] = '\0';

        // Unixodbc whines about 'Driver=' but connects anyway.
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "TestBase: SQLDriverConnect " << connectString
                      << " failed: " << getDiagText(SQL_HANDLE_DBC, dbc)
                      << " retcode: " << ret
                      << std::endl;

            exit(EXIT_FAILURE);
        }
    }

#if !defined(ODBC_WINDOWS)

    void reportInstallerErrors() {
        RETCODE rc = SQL_SUCCESS;
        char    message[SQL_MAX_MESSAGE_LENGTH];
        WORD    errCodeIn = 1;
        DWORD   errCodeOut = 0L;
        WORD    cbErrorMsg = 0;

        memset(message, 0, sizeof(message));
        while (rc == SQL_SUCCESS) {
            rc = SQLInstallerError(errCodeIn, &errCodeOut, message, sizeof(message) - 1, &cbErrorMsg);
            if (rc == SQL_SUCCESS) {
                fprintf(stderr, "%u %s\n", errCodeOut, message);
            }
            errCodeIn++;
        }
    }

    void append(std::vector<char>& v, char const* s) {
        v.insert(v.end(), s, s + strlen(s));
    }

    std::vector<char> setupDriverKeys(const char* driverPath)
    {
        std::vector<char> v;
        append(v, DRIVER_FULL_NAME);
        v.push_back(0);
        append(v, INSTALL_DRIVER);
        v.push_back('=');
        append(v, driverPath);
        v.push_back(0);
        append(v, INSTALL_API_LEVEL);
        v.push_back('=');
        append(v, VALUE_API_LEVEL);
        v.push_back(0);
        append(v, INSTALL_CONNECT_FUN);
        v.push_back('=');
        append(v, VALUE_CONNECT_FUN);
        v.push_back(0);
        append(v, INSTALL_FILE_USAGE);
        v.push_back('=');
        append(v, VALUE_FILE_USAGE);
        v.push_back(0);
        append(v, INSTALL_DRIVER_VER);
        v.push_back('=');
        append(v, VALUE_DRIVER_VER);
        v.push_back(0);
        append(v, INSTALL_SQL_LEVEL);
        v.push_back('=');
        append(v, VALUE_SQL_LEVEL);
        v.push_back(0);
        v.push_back(0);
        return v;
    }
#endif

    // assume skinny chars until we revamp this after W-ing the driver.
    std::string getCharData(int index, bool& nullValue)
    {
        nullValue = false;
        char colbuf[PRETTY_BIG];
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_CHAR, colbuf, PRETTY_BIG-1, &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData faild (char): " << getDiagText(SQL_HANDLE_STMT, stmt);
        if (ret != SQL_SUCCESS) {
            return std::string();
        }
        if (len == -1) {
            nullValue = true;
            return std::string();
        }
        return std::string(colbuf, len); // "" won't match anything
    }

    SQL_TIMESTAMP_STRUCT getTimestamp(int index, bool& nullValue)
    {
        nullValue = false;
        SQL_TIMESTAMP_STRUCT val;
        memset(&val, 0, sizeof(val));
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_TYPE_TIMESTAMP, &val, sizeof(val), &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData failed (timestamp)" << getDiagText(SQL_HANDLE_STMT, stmt);
        if (len == -1) {
            nullValue = true;
        }
        return val;
    }

    SQL_DATE_STRUCT getDate(int index, bool& nullValue)
    {
        nullValue = false;
        SQL_DATE_STRUCT val;
        memset(&val, 0, sizeof(val));
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_TYPE_DATE, &val, sizeof(val), &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData failed (date)" << getDiagText(SQL_HANDLE_STMT, stmt);
        if (len == -1) {
            nullValue = true;
        }
        return val;
    }

    SQL_TIME_STRUCT getTime(int index, bool& nullValue)
    {
        nullValue = false;
        SQL_TIME_STRUCT val;
        memset(&val, 0, sizeof(val));
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_TYPE_TIME, &val, sizeof(val), &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData failed (time)" << getDiagText(SQL_HANDLE_STMT, stmt);
        if (len == -1) {
            nullValue = true;
        }
        return val;
    }

    SQLINTEGER getIntData(int index)
    {
        SQLINTEGER val = 0;
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_SLONG, &val, sizeof(val), &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData failed (int)" << getDiagText(SQL_HANDLE_STMT, stmt);
        return val;
    }

    double getDoubleData(int index, bool& nullValue)
    {
        nullValue = false;
        double val = 0;
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_DOUBLE, &val, sizeof(val), &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData failed (double)" << getDiagText(SQL_HANDLE_STMT, stmt);
        if (len == -1) {
            nullValue = true;
        }
        return val;
    }

    SQLINTEGER getIntData(int index, bool& nullValue)
    {
        nullValue = false;
        SQLINTEGER val = 0;
        SQLLEN len = 0;
        RETCODE ret = SQLGetData(stmt, index, SQL_C_SLONG, &val, sizeof(val), &len);
        EXPECT_EQ(SQL_SUCCESS, ret) << "SQLGetData failed (int, null)" << getDiagText(SQL_HANDLE_STMT, stmt);
        if (len == -1) {
            nullValue = true;
        }
        return val;
    }


    void takeColAttr(ColAttributes& result, int index, int field, std::string& target) {
        char buffer[PRETTY_BIG];
        SQLSMALLINT num = 0;
        RETCODE ret = SQLColAttribute(stmt, index, field, buffer, PRETTY_BIG-1, &num, NULL);
        if (ret == SQL_SUCCESS) {
            result.valid.insert(field);
            buffer[num] = 0;
            target = std::string(buffer);
        }
    }

    void takeColAttr(ColAttributes& result, int index, int field, SQLLEN& target) {
        SQLLEN num = 0;
        RETCODE ret = SQLColAttribute(stmt, index, field, NULL, 0, NULL, &num);
        if (ret == SQL_SUCCESS) {
            result.valid.insert(field);
            target = num;
        }
    }

    void takeColAttr(ColAttributes& result, int index, int field, bool& target) {
        SQLLEN num;
        RETCODE ret = SQLColAttribute(stmt, index, field, NULL, 0, NULL, &num);
        if (ret == SQL_SUCCESS) {
            result.valid.insert(field);
            target = num == SQL_TRUE;
        }
    }

    ColAttributes colAttributes(int index) {
        ColAttributes result;

        takeColAttr(result, index, SQL_DESC_AUTO_UNIQUE_VALUE, result.autoUniqueValue);
        takeColAttr(result, index, SQL_DESC_BASE_COLUMN_NAME, result.baseColumnName);
        takeColAttr(result, index, SQL_DESC_BASE_TABLE_NAME, result.baseColumnName);
        takeColAttr(result, index, SQL_DESC_CASE_SENSITIVE, result.caseSensitive);
        takeColAttr(result, index, SQL_DESC_CATALOG_NAME, result.catalogName);
        takeColAttr(result, index, SQL_DESC_CONCISE_TYPE, result.conciseType);
        takeColAttr(result, index, SQL_DESC_COUNT, result.count);
        takeColAttr(result, index, SQL_DESC_DISPLAY_SIZE, result.count);
        takeColAttr(result, index, SQL_DESC_FIXED_PREC_SCALE, result.fixedPrecScale);
        takeColAttr(result, index, SQL_DESC_LABEL, result.label);
        takeColAttr(result, index, SQL_DESC_LENGTH, result.length);
        takeColAttr(result, index, SQL_DESC_LITERAL_PREFIX, result.literalPrefix);
        takeColAttr(result, index, SQL_DESC_LITERAL_SUFFIX, result.literalSuffix);
        takeColAttr(result, index, SQL_DESC_LOCAL_TYPE_NAME, result.localTypeName);
        takeColAttr(result, index, SQL_DESC_NAME, result.name);
        takeColAttr(result, index, SQL_DESC_NULLABLE, result.nullable);
        takeColAttr(result, index, SQL_DESC_NUM_PREC_RADIX, result.numPrecRadix);
        takeColAttr(result, index, SQL_DESC_OCTET_LENGTH, result.octetLength);
        takeColAttr(result, index, SQL_DESC_PRECISION, result.precision);
        takeColAttr(result, index, SQL_DESC_SCALE, result.scale);
        takeColAttr(result, index, SQL_DESC_SCHEMA_NAME, result.schemaName);
        takeColAttr(result, index, SQL_DESC_SEARCHABLE, result.searchable);
        takeColAttr(result, index, SQL_DESC_TABLE_NAME, result.tableName);
        takeColAttr(result, index, SQL_DESC_UNNAMED, result.unnamed);
        takeColAttr(result, index, SQL_DESC_UNSIGNED, result.unsigned_);
        takeColAttr(result, index, SQL_DESC_UPDATABLE, result.updatable);

        return result;
    }

    void execDirect(const char* sql) {
        ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)sql, SQL_NTS))
            << getDiagText(SQL_HANDLE_STMT, stmt) << ":\n  " << sql;
    }

    void fetch() {
        ASSERT_EQ(SQL_SUCCESS, SQLFetch(stmt)) << "SQLFetch failed";
    }

    void execDirectAndFetch(const char* sql) {
        execDirect(sql);
        fetch();
    }

    void freeStmt() {
        ASSERT_EQ(SQL_SUCCESS, SQLFreeStmt(stmt, SQL_CLOSE)) << "SQLFreeStmt failed";
    }

    std::string getDiagText(SQLSMALLINT handleType, SQLHANDLE handle) {
        SQLCHAR txt[1024];
        SQLCHAR state[6];
        SQLINTEGER err;
        SQLSMALLINT len;
        if (SQLGetDiagRec(handleType, handle, 1, state, &err, txt, 1023, &len) != SQL_SUCCESS) {
            return "Failed to retrieve diag record";
        }

        txt[len] = '\0';
        return std::string((char*)txt);
    }

    void createComplianceData()
    {
        // no 'if exists' for drop schema.
        SQLExecDirect(stmt, (SQLCHAR*)"drop schema compliance cascade", SQL_NTS);
        execDirect("create schema compliance");
        execDirect("use compliance");
        execDirect("create table person (first_name string)");
        execDirect("create table autogen (id bigint generated always as identity primary key, name string)");
        execDirect("create table types (c_smallint smallint not null, c_int int not null, c_bigint bigint not null, c_float float not null, c_double double not null, c_date date not null, c_timestamp timestamp not null, c_time time not null, c_clob clob, c_blob blob, c_numeric numeric not null, c_number number not null, c_bignumeric numeric(100) not null, c_string varchar(255) not null, c_bytes bytes not null, c_bool boolean not null)");
        execDirect("create table numerictype (c_numeric numeric not null)");
        execDirect("create table numbertype (c_number number not null)");
        execDirect("create table bignumerictype (c_bignumeric numeric(121) not null)");
        execDirect("create table strsizes(v1 varchar(1), v11 varchar(11), v111 varchar(111), vstring string)");
        execDirect("create table fixedtab(f1 int, f2 int, f3 int)");
        execDirect("create table booltype(c_bool boolean)");
        execDirect("create table inttype(c_int int)");

        execDirect("create procedure proc1(in p1 string, out p2 double, inout p3 date) "
                   "returns tbl(col1 time, col2 datetime, col3 number, col4 decimal(3)) "
                   "as end_procedure");
        execDirect("create function func1(p1 string, p2 double, p3 date) "
                   "returns table tbl(col1 time, col2 datetime, col3 number, col4 decimal(3)) "
                   "as end_function");

        execDirect("insert into person (first_name) values('stephen')");
        execDirect("insert into person (first_name) values('james')");
        execDirect("insert into person (first_name) values('roger')");
        execDirect("insert into types (c_smallint, c_int,c_bigint, c_float, c_double, c_date,c_timestamp,c_time, c_numeric, c_number, c_bignumeric, c_string, c_bytes, c_bool) values (11, 1002, 1003, 1004, 1005, '01/01/2000','2000-01-01 12:12:12.0','12:12:12', 1006, 1007, 1008, '1009', '1010', true)");
        execDirect("insert into numbertype(c_number) values (1001)");
        execDirect("insert into numerictype(c_numeric) values (1002)");
        execDirect("insert into bignumerictype(c_bignumeric) values (1003)");
        execDirect("insert into autogen (name) values ('stephen')");
        execDirect("insert into strsizes (v1,v11,v111,vstring) values ('o', 'one', 'oneoneoneone', 'oneoneoneoneone')");
        execDirect("insert into strsizes (v1,v11,v111,vstring) values ('t', 'two', 'twotwotwotwo', 'twotwotwotwotwo')");
        execDirect("insert into strsizes (v1,v11,v111,vstring) values ('3', 'threee', 'threeethreeethreeethreee', 'threeethreeethreeethreeethreee')");
        execDirect("insert into fixedtab(f1, f2, f3) values(1,1,1)");
        execDirect("insert into fixedtab(f1, f2, f3) values(2,2,2)");
        execDirect("insert into fixedtab(f1, f2, f3) values(3,3,3)");
        execDirect("insert into fixedtab(f1, f2, f3) values(4,4,4)");
    }

    SQLHENV     henv;
    SQLHDBC     hdbc1;
    SQLHSTMT    stmt;
};

inline bool operator==(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2)
{
    return ts1.day == ts2.day
        && ts1.fraction == ts2.fraction
        && ts1.hour == ts2.hour
        && ts1.minute == ts2.minute
        && ts1.month == ts2.month
        && ts1.second == ts2.second
        && ts1.year == ts2.year;

}

inline bool operator==(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2)
{
    return d1.day == d2.day
        && d1.month == d2.month
        && d1.year == d2.year;
}

inline bool operator==(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2)
{
    return t1.hour == t2.hour
        && t1.minute == t2.minute
        && t1.second == t2.second;

}
