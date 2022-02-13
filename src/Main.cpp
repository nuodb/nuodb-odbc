/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "OdbcBase.h"

#include <stdio.h>
#include <stdarg.h>

#include <odbcinst.h>

#include "OdbcTrace.h"
#include "OdbcEnv.h"
#include "OdbcConnection.h"
#include "OdbcStatement.h"

#include "NuoRemote/CallableStatement.h"
#include "NuoRemote/Connection.h"
#include "NuoRemote/DatabaseMetaData.h"
#include "NuoRemote/PreparedStatement.h"
#include "NuoRemote/ResultSet.h"
#include "NuoRemote/ResultSetMetaData.h"

// For some strange reason the ODBC function definitions in Windows <sql.h>
// are not dllexported.  Adding it here causes MSVC to throw a fit due to
// "different linkage" errors.  NuoODBC.def deals with those.
#define NUODB_ODBCAPI __attribute__((visibility("default")))

#define PROTECT_SQLLEN(x) ((x == SQL_NTS)? x : (x & 0xFFFFFFFF))

// Debugging
#if defined(NUOODBC_DEBUG)
static FILE* dbg;

static const char* dbg_file = "nuoodbc_debug.log";

static int DebugInit()
{
    char buf[1024];
    const char* tmpdir = getenv("NUOODBC_TEST_TEMP");
#if defined(_WIN32)
    const char* def = ".";
    const char* sep = "\\";
    if (!tmpdir) {
        tmpdir = getenv("TEMP");
    }
#else
    const char* def = "/tmp";
    const char* sep = "/";
    if (!tmpdir) {
        tmpdir = getenv("TMPDIR");
    }
#endif
    if (!tmpdir) {
        tmpdir = def;
    }
    sprintf(buf, "%s%s%s", tmpdir, sep, dbg_file);
    dbg = fopen(buf, "w");
    if (dbg) {
        fprintf(dbg, ">> Configuring NuoDB ODBC Driver\n");
        fflush(dbg);
    }
    return 1;
}

static int _debugLoader = 0;

void nuoodbc_debug(const char* msg, ...)
{
    if (!_debugLoader) {
        _debugLoader = DebugInit();
    }
    if (dbg) {
        va_list args;
        va_start(args, msg);
        vfprintf(dbg, msg, args);
        fputc('\n', dbg);
        fflush(dbg);
        va_end(args);
    }
}

#else

void nuoodbc_debug(const char* msg __attribute__((unused)))
{}

#endif

// Private symbols for our implementations, if we call these ourselves
// without the underscore they could get bound to odbc.so's definition
// of these symbols, mostly a unix problem.

static RETCODE _SQLFetch(HSTMT arg0);
static RETCODE _SQLAllocHandle(SQLSMALLINT arg0, SQLHANDLE arg1, SQLHANDLE* arg2);
static RETCODE _SQLEndTran(SQLSMALLINT arg0, SQLHANDLE arg1, SQLSMALLINT arg2);

///// SQLAllocConnect /////
RETCODE NUODB_ODBCAPI SQL_API SQLAllocConnect(HENV arg0,
                                              HDBC* arg1)
{
    TRACE("SQLAllocConnect");

    RETCODE retcode = _SQLAllocHandle(SQL_HANDLE_DBC, arg0, arg1);
    TRACERET("SQLAllocConnect", retcode);
    return retcode;
}

///// SQLAllocEnv /////

RETCODE NUODB_ODBCAPI SQL_API SQLAllocEnv(HENV* arg0)
{
    TRACE("SQLAllocEnv");

    RETCODE retcode = _SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, arg0);
    TRACERET("SQLAllocEnv", retcode);
    return retcode;
}

///// SQLAllocStmt /////

RETCODE NUODB_ODBCAPI SQL_API SQLAllocStmt(HDBC arg0,
                                           HSTMT* arg1)
{
    TRACE("SQLAllocStmt");

    RETCODE retcode = _SQLAllocHandle(SQL_HANDLE_STMT, arg0, arg1);
    TRACERET("SQLAllocStmt", retcode);
    return retcode;
}

///// SQLBindCol /////

RETCODE NUODB_ODBCAPI SQL_API SQLBindCol(HSTMT arg0,
                                         SQLUSMALLINT arg1,
                                         SQLSMALLINT arg2,
                                         SQLPOINTER arg3,
                                         SQLLEN arg4,
                                         SQLLEN* arg5)
{
    TRACE("SQLBindCol");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlBindCol(arg1, arg2, arg3, PROTECT_SQLLEN(arg4), arg5);
    TRACERET("SQLBindCol", retcode);
    return retcode;
}

///// SQLCancel /////

RETCODE NUODB_ODBCAPI SQL_API SQLCancel(HSTMT arg0)
{
    TRACE("SQLCancel");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlCancel();
    TRACERET("SQLCancel", retcode);
    return retcode;
}

///// SQLColAttributes /////

RETCODE NUODB_ODBCAPI SQL_API SQLColAttributes(HSTMT arg0,
                                               SQLUSMALLINT arg1,
                                               SQLUSMALLINT arg2,
                                               SQLPOINTER arg3,
                                               SQLSMALLINT arg4,
                                               SQLSMALLINT* arg5,
                                               SQLLEN* arg6)
{
    TRACE("SQLColAttributes");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlColAttributes(arg1, arg2, arg3, arg4, arg5, arg6);
    TRACERET("SQLColAttributes", retcode);
    return retcode;
}

///// SQLColAttribute /////

RETCODE NUODB_ODBCAPI SQL_API SQLColAttribute(SQLHSTMT handle,
                                              SQLUSMALLINT columnNumber,
                                              SQLUSMALLINT fieldIdentifier,
                                              SQLPOINTER characterAttrPtr,
                                              SQLSMALLINT bufferLen,
                                              SQLSMALLINT* stringLengthPtr,
                                              SQLLEN* numericAttrPtr
    )
{
    TRACE(formatString("SQLColAttribute: column %u attr %u", columnNumber, fieldIdentifier).c_str());
    RETCODE retcode = ((OdbcStatement*)handle)->sqlColAttribute(columnNumber, fieldIdentifier, characterAttrPtr, bufferLen, stringLengthPtr, numericAttrPtr);
    TRACERET("SQLColAttribute", retcode);
    return retcode;
}

///// SQLConnect /////

RETCODE NUODB_ODBCAPI SQL_API SQLConnect(HDBC arg0,
                                         SQLCHAR* arg1,
                                         SQLSMALLINT arg2,
                                         SQLCHAR* arg3,
                                         SQLSMALLINT arg4,
                                         SQLCHAR* arg5,
                                         SQLSMALLINT arg6)
{
    TRACE("SQLConnect");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlConnect(arg1, arg2, arg3, arg4, arg5, arg6);
    TRACERET("SQLConnect", retcode);
    return retcode;
}

///// SQLDescribeCol /////

RETCODE NUODB_ODBCAPI SQL_API SQLDescribeCol(HSTMT arg0,
                                             SQLUSMALLINT arg1,
                                             SQLCHAR* arg2,
                                             SQLSMALLINT arg3,
                                             SQLSMALLINT* arg4,
                                             SQLSMALLINT* arg5,
                                             SQLULEN* arg6,
                                             SQLSMALLINT* arg7,
                                             SQLSMALLINT* arg8)
{
    TRACE("SQLDescribeCol");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlDescribeCol(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    TRACERET("SQLDescribeCol", retcode);
    return retcode;
}

///// SQLDisconnect /////

RETCODE NUODB_ODBCAPI SQL_API SQLDisconnect(HDBC arg0)
{
    TRACE("SQLDisconnect");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlDisconnect();
    TRACERET("SQLDisconnect", retcode);
    return retcode;
}

///// SQLError /////

RETCODE NUODB_ODBCAPI SQL_API SQLError(SQLHENV env,
                                       SQLHDBC connection,
                                       SQLHSTMT statement,
                                       SQLCHAR* sqlState,
                                       SQLINTEGER* nativeErrorCode,
                                       SQLCHAR* msgBuffer,
                                       SQLSMALLINT msgBufferLength,
                                       SQLSMALLINT* msgLength)
{
    TRACE("SQLError");

    if (statement) {
        return ((OdbcStatement*)statement)->sqlError(sqlState, nativeErrorCode, msgBuffer, msgBufferLength, msgLength);
    }

    if (connection) {
        return ((OdbcConnection*)connection)->sqlError(sqlState, nativeErrorCode, msgBuffer, msgBufferLength, msgLength);
    }

    if (env) {
        return ((OdbcEnv*)env)->sqlError(sqlState, nativeErrorCode, msgBuffer, msgBufferLength, msgLength);
    }

    return SQL_ERROR;
}

///// SQLExecDirect /////

RETCODE NUODB_ODBCAPI SQL_API SQLExecDirect(HSTMT arg0,
                                            SQLCHAR* arg1,
                                            SQLINTEGER arg2)
{
    TRACE("SQLExecDirect");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlExecuteDirect(arg1, arg2);
    TRACERET("SQLExecDirect", retcode);
    return retcode;
}

///// SQLExecute /////

RETCODE NUODB_ODBCAPI SQL_API SQLExecute(HSTMT arg0)
{
    TRACE("SQLExecute");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlExecute();
    TRACERET("SQLExecute", retcode);
    return retcode;
}

///// SQLFetch /////

RETCODE NUODB_ODBCAPI SQL_API SQLFetch(HSTMT arg0)
{
    return _SQLFetch(arg0);
}

static RETCODE _SQLFetch(HSTMT arg0)
{
    TRACE("SQLFetch");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlFetch();
    TRACERET("SQLFetch", retcode);
    return retcode;
}

///// SQLFreeConnect /////

RETCODE NUODB_ODBCAPI SQL_API SQLFreeConnect(HDBC arg0)
{
    TRACE("SQLFreeConnect");

    RETCODE retcode = SQLFreeHandle(SQL_HANDLE_DBC, arg0);
    TRACERET("SQLFreeconnect", retcode);
    return retcode;
}

///// SQLFreeEnv /////

RETCODE NUODB_ODBCAPI SQL_API SQLFreeEnv(HENV arg0)
{
    TRACE("SQLFreeEnv");

    RETCODE retcode = SQLFreeHandle(SQL_HANDLE_ENV, arg0);
    TRACERET("SQLFreeEnv", retcode);
    return retcode;
}

///// SQLFreeStmt /////

RETCODE NUODB_ODBCAPI SQL_API SQLFreeStmt(HSTMT arg0,
                                          SQLUSMALLINT arg1)
{
    TRACE("SQLFreeStmt");

    if (arg1 == SQL_DROP) {
        RETCODE retcode = SQLFreeHandle(SQL_HANDLE_STMT, arg0);
        TRACERET("SQLFreeStmt freehandle", retcode);
        return retcode;
    }

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlFreeStmt(arg1);
    TRACERET("SQLFreeStmt", retcode);
    return retcode;
}

///// SQLNumResultCols /////

RETCODE NUODB_ODBCAPI SQL_API SQLNumResultCols(HSTMT arg0, SQLSMALLINT* arg1)
{
    TRACE("SQLNumResultCols");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlNumResultCols(arg1);
    TRACERET("SQLNumResultCols", retcode);
    return retcode;
}

///// SQLPrepare /////

RETCODE NUODB_ODBCAPI SQL_API SQLPrepare(HSTMT arg0,
                                         SQLCHAR* arg1,
                                         SQLINTEGER arg2)
{
    TRACE("SQLPrepare");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlPrepare(arg1, arg2);
    TRACERET("SQLPrepare", retcode);
    return retcode;
}

///// SQLRowCount /////

RETCODE NUODB_ODBCAPI SQL_API SQLRowCount(HSTMT arg0, SQLLEN* arg1)
{
    TRACE("SQLRowCount");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlRowCount(arg1);
    TRACERET("SQLRowCount", retcode);
    return retcode;
}

///// SQLGetCursorName /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetCursorName(HSTMT arg0,
                                               SQLCHAR* arg1,
                                               SQLSMALLINT arg2,
                                               SQLSMALLINT* arg3)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLGetCursorName called");
}

///// SQLSetCursorName /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetCursorName(HSTMT arg0,
                                               SQLCHAR* arg1,
                                               SQLSMALLINT arg2)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLSetCursorName called");
}

///// SQLSetParam ///// Deprecated in 2.0

RETCODE NUODB_ODBCAPI SQL_API SQLSetParam(HSTMT arg0,
                                          SQLUSMALLINT arg1,
                                          SQLSMALLINT arg2,
                                          SQLSMALLINT arg3,
                                          SQLULEN arg4,
                                          SQLSMALLINT arg5,
                                          SQLPOINTER arg6,
                                          SQLLEN* arg7)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLSetParam called");
}

///// SQLTransact /////

RETCODE NUODB_ODBCAPI SQL_API SQLTransact(HENV arg0,
                                          HDBC arg1,
                                          SQLUSMALLINT arg2)
{
    TRACE("SQLTransact");

    if (arg0 == SQL_NULL_HDBC) {
        RETCODE retcode = _SQLEndTran(SQL_HANDLE_DBC, arg1, arg2);
        TRACERET("SQLTransact", retcode);
        return retcode;
    }

    RETCODE retcode = _SQLEndTran(SQL_HANDLE_ENV, arg0, arg2);
    TRACERET("SQLTransact", retcode);
    return retcode;
}

///// SQLColumns /////

RETCODE NUODB_ODBCAPI SQL_API SQLColumns(HSTMT arg0,
                                         SQLCHAR* arg1,
                                         SQLSMALLINT arg2,
                                         SQLCHAR* arg3,
                                         SQLSMALLINT arg4,
                                         SQLCHAR* arg5,
                                         SQLSMALLINT arg6,
                                         SQLCHAR* arg7,
                                         SQLSMALLINT arg8)
{
    TRACE("SQLColumns");
    RETCODE retcode = ((OdbcStatement*)arg0)->sqlColumns(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    TRACERET("SQLColumns", retcode);
    return retcode;
}

///// SQLDriverConnect /////

RETCODE NUODB_ODBCAPI SQL_API SQLDriverConnect(HDBC arg0,
                                               HWND hWnd,
                                               UCHAR* szConnStrIn,
                                               SWORD cbConnStrIn,
                                               UCHAR* szConnStrOut,
                                               SWORD cbConnStrOut,
                                               SQLSMALLINT* pcbConnStrOut,
                                               UWORD uwMode)
{
    TRACE("SQLDriverConnect");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlDriverConnect(
        hWnd, szConnStrIn, cbConnStrIn,
        szConnStrOut, cbConnStrOut, pcbConnStrOut,
        uwMode);

    TRACERET("SQLDriverConnect", retcode);
    return retcode;
}

///// SQLGetConnectOption /////  Deprecated

RETCODE NUODB_ODBCAPI SQL_API SQLGetConnectOption(HDBC arg0,
                                                  SQLUSMALLINT arg1,
                                                  SQLPOINTER arg2)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLGetConnectOption called");
}

///// SQLGetData /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetData(HSTMT arg0,
                                         SQLUSMALLINT arg1,
                                         SQLSMALLINT arg2,
                                         SQLPOINTER arg3,
                                         SQLLEN arg4,
                                         SQLLEN* arg5)
{
    TRACE("SQLGetData");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlGetData(arg1, arg2, arg3, PROTECT_SQLLEN(arg4), arg5);
    TRACERET(formatString("SQLGetData with length " SQLLEN_FMT, arg5 ? *arg5 : (SQLLEN)-1).c_str(), retcode);
    return retcode;
}

///// SQLGetFunctions /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetFunctions(HDBC arg0,
                                              SQLUSMALLINT arg1,
                                              SQLUSMALLINT* arg2)
{
    TRACE("SQLGetFunctions");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlGetFunctions(arg1, arg2);
    TRACERET("SQLGetFunctions", retcode);
    return retcode;
}

///// SQLGetInfo /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetInfo(HDBC arg0,
                                         SQLUSMALLINT arg1,
                                         SQLPOINTER arg2,
                                         SQLSMALLINT arg3,
                                         SQLSMALLINT* arg4)
{
    TRACE("SQLGetInfo");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlGetInfo(arg1, arg2, arg3, arg4);
    TRACERET("SQLGetInfo", retcode);
    return retcode;
}

///// SQLGetStmtOption /////  Deprecated

RETCODE NUODB_ODBCAPI SQL_API SQLGetStmtOption(HSTMT arg0,
                                               SQLUSMALLINT arg1,
                                               SQLPOINTER arg2)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLGetStmtOption called");
}

///// SQLGetTypeInfo /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetTypeInfo(HSTMT arg0,
                                             SQLSMALLINT arg1)
{
    TRACE("SQLGetTypeInfo");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlGetTypeInfo(arg1);
    TRACERET("SQLGetTypeInfo", retcode);
    return retcode;
}

///// SQLParamData /////

RETCODE NUODB_ODBCAPI SQL_API SQLParamData(HSTMT arg0,
                                           SQLPOINTER* arg1)
{
    TRACE("SQLParamData");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlParamData(arg1);
    TRACERET("SQLParamData", retcode);
    return retcode;
}

///// SQLPutData /////

RETCODE NUODB_ODBCAPI SQL_API SQLPutData(HSTMT arg0,
                                         SQLPOINTER arg1,
                                         SQLLEN arg2)
{
    TRACE("SQLPutData");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlPutData(arg1, PROTECT_SQLLEN(arg2));
    TRACERET("SQLPutData", retcode);
    return retcode;
}

///// SQLSetConnectOption ///// Deprecated

RETCODE NUODB_ODBCAPI SQL_API SQLSetConnectOption(HDBC arg0,
                                                  SQLUSMALLINT arg1,
                                                  SQLULEN arg2)
{
    TRACE("SQLSetConnectOption");

    RETCODE retcode = SQLSetConnectAttr(arg0, arg1, (SQLPOINTER)arg2, 0);
    TRACERET("SQLSetConnectOption", retcode);
    return retcode;
}

///// SQLSetStmtOption ///// Deprecated

RETCODE NUODB_ODBCAPI SQL_API SQLSetStmtOption(HSTMT arg0,
                                               SQLUSMALLINT arg1,
                                               SQLULEN arg2)
{
    TRACE("SQLSetStmtOption");
    RETCODE retcode;

    switch (arg1) {
        case SQL_NOSCAN:
            // TODO: Communicate this setting to the server.
            retcode = SQL_SUCCESS;
            break;
        default:
            retcode = notYetImplemented((OdbcObject*)arg0, "SQLSetStmtOption   called");
    }

    TRACERET("SQLSetStmtOption", retcode);
    return retcode;
}

///// SQLSpecialColumns /////

RETCODE NUODB_ODBCAPI SQL_API SQLSpecialColumns(HSTMT arg0,
                                                SQLUSMALLINT arg1,
                                                SQLCHAR* arg2,
                                                SQLSMALLINT arg3,
                                                SQLCHAR* arg4,
                                                SQLSMALLINT arg5,
                                                SQLCHAR* arg6,
                                                SQLSMALLINT arg7,
                                                SQLUSMALLINT arg8,
                                                SQLUSMALLINT arg9)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLSpecialColumns called");
}

///// SQLStatistics /////

RETCODE NUODB_ODBCAPI SQL_API SQLStatistics(HSTMT arg0,
                                            SQLCHAR* arg1,
                                            SQLSMALLINT arg2,
                                            SQLCHAR* arg3,
                                            SQLSMALLINT arg4,
                                            SQLCHAR* arg5,
                                            SQLSMALLINT arg6,
                                            SQLUSMALLINT arg7,
                                            SQLUSMALLINT arg8)
{
    TRACE("SQLStatistics");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlStatistics(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    TRACERET("SQLStatistics", retcode);
    return retcode;
}

///// SQLTables /////

RETCODE NUODB_ODBCAPI SQL_API SQLTables(HSTMT arg0,
                                        SQLCHAR* arg1,
                                        SQLSMALLINT arg2,
                                        SQLCHAR* arg3,
                                        SQLSMALLINT arg4,
                                        SQLCHAR* arg5,
                                        SQLSMALLINT arg6,
                                        SQLCHAR* arg7,
                                        SQLSMALLINT arg8)
{
    TRACE("SQLTables");
    RETCODE retcode = ((OdbcStatement*)arg0)->sqlTables(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    TRACERET("SQLTables", retcode);
    return retcode;
}

///// SQLBrowseConnect /////

RETCODE NUODB_ODBCAPI SQL_API SQLBrowseConnect(SQLHDBC connectionHandle,
                                               SQLCHAR* inConnectionString,
                                               SQLSMALLINT stringLength1,
                                               SQLCHAR* outConnectionString,
                                               SQLSMALLINT bufferLength,
                                               SQLSMALLINT* stringLength2Ptr)
{
    TRACE(formatString("SQLBrowseConnect with (%s)", std::string((const char*)inConnectionString, stringLength1).c_str()).c_str());
    RETCODE retcode = ((OdbcConnection*)connectionHandle)->sqlBrowseConnect(inConnectionString,
                                                                            stringLength1,
                                                                            outConnectionString,
                                                                            bufferLength,
                                                                            stringLength2Ptr);
    TRACERET("SQLBrowseConnect", retcode);
    return retcode;
}

///// SQLColumnPrivileges /////

RETCODE NUODB_ODBCAPI SQL_API SQLColumnPrivileges(HSTMT arg0,
                                                  SQLCHAR* arg1,
                                                  SQLSMALLINT arg2,
                                                  SQLCHAR* arg3,
                                                  SQLSMALLINT arg4,
                                                  SQLCHAR* arg5,
                                                  SQLSMALLINT arg6,
                                                  SQLCHAR* arg7,
                                                  SQLSMALLINT arg8)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLColumnPrivileges called");
}

///// SQLDataSources /////

RETCODE NUODB_ODBCAPI SQL_API SQLDataSources(HENV arg0,
                                             SQLUSMALLINT arg1,
                                             SQLCHAR* arg2,
                                             SQLSMALLINT arg3,
                                             SQLSMALLINT* arg4,
                                             SQLCHAR* arg5,
                                             SQLSMALLINT arg6,
                                             SQLSMALLINT* arg7)
{
    notYetImplemented("SQLDataSources called");
    return SQL_SUCCESS;
}

///// SQLDescribeParam /////

RETCODE NUODB_ODBCAPI SQL_API SQLDescribeParam(HSTMT arg0,
                                               SQLUSMALLINT arg1,
                                               SQLSMALLINT* arg2,
                                               SQLULEN* arg3,
                                               SQLSMALLINT* arg4,
                                               SQLSMALLINT* arg5)
{
    TRACE("SQLDescribeParam");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlDescribeParam(arg1, arg2, arg3, arg4, arg4);
    TRACERET("SQLDescribeParam", retcode);
    return retcode;
}

///// SQLExtendedFetch /////

RETCODE NUODB_ODBCAPI SQL_API SQLExtendedFetch(HSTMT arg0,
                                               SQLUSMALLINT arg1,
                                               SQLLEN arg2,
                                               SQLULEN* arg3,
                                               SQLUSMALLINT* arg4)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLExtendedFetch called");
}

///// SQLForeignKeys /////

RETCODE NUODB_ODBCAPI SQL_API SQLForeignKeys(HSTMT arg0,
                                             SQLCHAR* arg1,
                                             SQLSMALLINT arg2,
                                             SQLCHAR* arg3,
                                             SQLSMALLINT arg4,
                                             SQLCHAR* arg5,
                                             SQLSMALLINT arg6,
                                             SQLCHAR* arg7,
                                             SQLSMALLINT arg8,
                                             SQLCHAR* arg9,
                                             SQLSMALLINT arg10,
                                             SQLCHAR* arg11,
                                             SQLSMALLINT arg12)
{
    return ((OdbcStatement*)arg0)->sqlForeignKeys(arg1, arg2, arg3, arg4,
                                                  arg5, arg6, arg7, arg8,
                                                  arg9, arg10, arg11, arg12);
}

///// SQLMoreResults /////

RETCODE NUODB_ODBCAPI SQL_API SQLMoreResults(HSTMT arg0)
{
    TRACE("SQLMoreResults");
    RETCODE retcode = ((OdbcStatement*)arg0)->sqlMoreResults();
    TRACERET("SQLMoreResults", retcode);
    return retcode;
}

///// SQLNativeSql /////

RETCODE NUODB_ODBCAPI SQL_API SQLNativeSql(HDBC arg0,
                                           SQLCHAR* arg1,
                                           SQLINTEGER arg2,
                                           SQLCHAR* arg3,
                                           SQLINTEGER arg4,
                                           SQLINTEGER* arg5)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLNativeSql called");
}

///// SQLNumParams /////

RETCODE NUODB_ODBCAPI SQL_API SQLNumParams(HSTMT arg0,
                                           SQLSMALLINT* arg1)
{
    return ((OdbcStatement*)arg0)->sqlNumParameters(arg1);
}

///// SQLParamOptions /////

RETCODE NUODB_ODBCAPI SQL_API SQLParamOptions(HSTMT arg0,
                                              SQLULEN arg1,
                                              SQLULEN* arg2)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLParamOptions called");
}

///// SQLPrimaryKeys /////

RETCODE NUODB_ODBCAPI SQL_API SQLPrimaryKeys(HSTMT arg0,
                                             SQLCHAR* arg1,
                                             SQLSMALLINT arg2,
                                             SQLCHAR* arg3,
                                             SQLSMALLINT arg4,
                                             SQLCHAR* arg5,
                                             SQLSMALLINT arg6)
{
    TRACE("SQLPrimaryKeys");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlPrimaryKeys(arg1, arg2, arg3, arg4, arg5, arg6);
    TRACERET("SQLPrimaryKeys", retcode);
    return retcode;
}

///// SQLProcedureColumns /////

RETCODE NUODB_ODBCAPI SQL_API SQLProcedureColumns(HSTMT arg0,
                                                  SQLCHAR* arg1,
                                                  SQLSMALLINT arg2,
                                                  SQLCHAR* arg3,
                                                  SQLSMALLINT arg4,
                                                  SQLCHAR* arg5,
                                                  SQLSMALLINT arg6,
                                                  SQLCHAR* arg7,
                                                  SQLSMALLINT arg8)
{
    TRACE("SQLProcedureColumns");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlProcedureColumns(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    TRACERET("SQLProcedureColumns", retcode);
    return retcode;
}

///// SQLProcedures /////

RETCODE NUODB_ODBCAPI SQL_API SQLProcedures(HSTMT arg0,
                                            SQLCHAR* arg1,
                                            SQLSMALLINT arg2,
                                            SQLCHAR* arg3,
                                            SQLSMALLINT arg4,
                                            SQLCHAR* arg5,
                                            SQLSMALLINT arg6)
{
    TRACE("SQLProcedures");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlProcedures(arg1, arg2, arg3, arg4, arg5, arg6);
    TRACERET("SQLProcedures", retcode);
    return retcode;
}

///// SQLSetPos /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetPos(HSTMT arg0,
                                        SQLSETPOSIROW arg1,
                                        SQLUSMALLINT arg2,
                                        SQLUSMALLINT arg3)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLSetPos called");
}

///// SQLSetScrollOptions /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetScrollOptions(HSTMT arg0,
                                                  SQLUSMALLINT arg1,
                                                  SQLLEN arg2,
                                                  SQLUSMALLINT arg3)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLSetScrollOptions called");
}

///// SQLTablePrivileges /////

RETCODE NUODB_ODBCAPI SQL_API SQLTablePrivileges(HSTMT arg0,
                                                 SQLCHAR* arg1,
                                                 SQLSMALLINT arg2,
                                                 SQLCHAR* arg3,
                                                 SQLSMALLINT arg4,
                                                 SQLCHAR* arg5,
                                                 SQLSMALLINT arg6)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLTablePrivileges called");
}

///// SQLDrivers /////

RETCODE NUODB_ODBCAPI SQL_API SQLDrivers(HENV arg0,
                                         SQLUSMALLINT arg1,
                                         SQLCHAR* arg2,
                                         SQLSMALLINT arg3,
                                         SQLSMALLINT* arg4,
                                         SQLCHAR* arg5,
                                         SQLSMALLINT arg6,
                                         SQLSMALLINT* arg7)
{
    notYetImplemented("SQLDrivers called");
    return SQL_SUCCESS;
}

///// SQLBindParameter /////

RETCODE NUODB_ODBCAPI SQL_API SQLBindParameter(SQLHSTMT arg0,
                                               SQLUSMALLINT arg1,
                                               SQLSMALLINT arg2,
                                               SQLSMALLINT arg3,
                                               SQLSMALLINT arg4,
                                               SQLULEN arg5,
                                               SQLSMALLINT arg6,
                                               SQLPOINTER arg7,
                                               SQLLEN arg8,
                                               SQLLEN* arg9)
{
    TRACE("SQLBindParameter");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlBindParameter(arg1, arg2, arg3, arg4, arg5, arg6, arg7, PROTECT_SQLLEN(arg8), arg9);
    TRACERET("SQLBindParameter", retcode);
    return retcode;
}

///// SQLAllocHandle /////

RETCODE NUODB_ODBCAPI SQL_API SQLAllocHandle(SQLSMALLINT arg0,
                                             SQLHANDLE arg1,
                                             SQLHANDLE* arg2)
{
    return _SQLAllocHandle(arg0, arg1, arg2);
}

static RETCODE _SQLAllocHandle(SQLSMALLINT arg0,
                               SQLHANDLE arg1,
                               SQLHANDLE* arg2)
{
    TRACE("SQLAllocHandle");

    if (arg0 == SQL_HANDLE_ENV) {
        if (arg1 != SQL_NULL_HANDLE || arg2 == NULL) {
            return SQL_ERROR;
        }

        *arg2 = new OdbcEnv;
        return SQL_SUCCESS;
    }

    OdbcObject* object = (OdbcObject*)arg1;

    if (arg2 == NULL) {
        return object->sqlReturn(SQL_ERROR, "HY009", "Invalid use of null pointer");
    }

    return object->allocHandle(arg0, arg2);
}

///// SQLBindParam /////

RETCODE NUODB_ODBCAPI SQL_API SQLBindParam(SQLHSTMT arg0,
                                           SQLUSMALLINT arg1,
                                           SQLSMALLINT arg2,
                                           SQLSMALLINT arg3,
                                           SQLULEN arg4,
                                           SQLSMALLINT arg5,
                                           SQLPOINTER arg6,
                                           SQLLEN* arg7)
{
    return notYetImplemented((OdbcObject*)arg0, "SQLBindParam called");
}

///// SQLCloseCursor /////

RETCODE NUODB_ODBCAPI SQL_API SQLCloseCursor(SQLHSTMT arg0)
{
    TRACE("SQLCloseCursor");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlCloseCursor();
    TRACERET("SQLCloseCursor", retcode);
    return retcode;
}

///// SQLCopyDesc /////

RETCODE NUODB_ODBCAPI SQL_API SQLCopyDesc(SQLHDESC arg0,
                                          SQLHDESC arg1)
{
    notYetImplemented("SQLCopyDesc called");
    return SQL_SUCCESS;
}

///// SQLEndTran /////

RETCODE NUODB_ODBCAPI SQL_API SQLEndTran(SQLSMALLINT arg0,
                                         SQLHANDLE arg1,
                                         SQLSMALLINT arg2)
{
    return _SQLEndTran(arg0, arg1, arg2);
}

static RETCODE _SQLEndTran(SQLSMALLINT arg0,
                           SQLHANDLE arg1,
                           SQLSMALLINT arg2)
{
    TRACE("SQLEndTran");

    switch (arg0) {
        case SQL_HANDLE_ENV: {
            RETCODE retcode = ((OdbcEnv*)arg1)->sqlEndTran(arg2);
            TRACERET("SQLEndTran", retcode);
            return retcode;
        }

        case SQL_HANDLE_DBC: {
            RETCODE retcode = ((OdbcConnection*)arg1)->sqlEndTran(arg2);
            TRACERET("SQLEndTran", retcode);
            return retcode;
        }
    }

    return SQL_INVALID_HANDLE;
}

///// SQLFetchScroll /////

RETCODE NUODB_ODBCAPI SQL_API SQLFetchScroll(SQLHSTMT handle,
                                             SQLSMALLINT fetchOrientation,
                                             SQLLEN fetchOffset)
{
    switch (fetchOrientation) {
        case SQL_FETCH_NEXT:
            return _SQLFetch(handle);

        default:
            return ((OdbcObject*)handle)->sqlReturn(SQL_ERROR, "HY106", "Invalid fetch type.  Only SQL_FETCH_NEXT supported");
    }
}

///// SQLFreeHandle /////

RETCODE NUODB_ODBCAPI SQL_API SQLFreeHandle(SQLSMALLINT arg0,
                                            SQLHANDLE arg1)
{
    TRACE("SQLFreeHandle");

    switch (arg0) {
        case SQL_HANDLE_ENV:
            delete (OdbcEnv*)arg1;
            break;

        case SQL_HANDLE_DBC:
            delete (OdbcConnection*)arg1;
            break;

        case SQL_HANDLE_STMT:
            delete (OdbcStatement*)arg1;
            break;

        case SQL_HANDLE_DESC:
            notYetImplemented("SQLFreeHandle DESC");

        default:
            RETCODE retcode = SQL_INVALID_HANDLE;
            TRACERET("SQLFreeHandle", retcode);
            return retcode;
    }

    return SQL_SUCCESS;
}

///// SQLGetConnectAttr /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetConnectAttr(SQLHDBC arg0,
                                                SQLINTEGER arg1,
                                                SQLPOINTER arg2,
                                                SQLINTEGER arg3,
                                                SQLINTEGER* arg4)
{
    TRACE("SQLGetConnectAttr");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlGetConnectAttr(arg1, arg2, arg3, arg4);
    TRACERET("SQLGetConnectAttr", retcode);
    return retcode;
}

///// SQLGetDescField /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetDescField(SQLHDESC arg0,
                                              SQLSMALLINT arg1,
                                              SQLSMALLINT arg2,
                                              SQLPOINTER arg3,
                                              SQLINTEGER arg4,
                                              SQLINTEGER* arg5)
{
    notYetImplemented("SQLGetDescField called");
    return SQL_SUCCESS;
}

///// SQLGetDescRec /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetDescRec(SQLHDESC arg0,
                                            SQLSMALLINT arg1,
                                            SQLCHAR* arg2,
                                            SQLSMALLINT arg3,
                                            SQLSMALLINT* arg4,
                                            SQLSMALLINT* arg5,
                                            SQLSMALLINT* arg6,
                                            SQLLEN* arg7,
                                            SQLSMALLINT* arg8,
                                            SQLSMALLINT* arg9,
                                            SQLSMALLINT* arg10)
{
    notYetImplemented("SQLGetDescRec called");
    return SQL_SUCCESS;
}

///// SQLGetDiagField /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetDiagField(SQLSMALLINT arg0,
                                              SQLHANDLE arg1,
                                              SQLSMALLINT arg2,
                                              SQLSMALLINT arg3,
                                              SQLPOINTER arg4,
                                              SQLSMALLINT arg5,
                                              SQLSMALLINT* arg6)
{
    TRACE("SQLGetDiagField");

    RETCODE retcode = ((OdbcObject*)arg1)->sqlGetDiagField(arg2, arg3, arg4, arg5, arg6);
    TRACERET("SQLGetDiagField", retcode);
    return retcode;
}

///// SQLGetDiagRec /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetDiagRec(SQLSMALLINT arg0,
                                            SQLHANDLE arg1,
                                            SQLSMALLINT arg2,
                                            SQLCHAR* arg3,
                                            SQLINTEGER* arg4,
                                            SQLCHAR* arg5,
                                            SQLSMALLINT arg6,
                                            SQLSMALLINT* arg7)
{
    TRACE("SQLGetDiagRec");

    RETCODE retcode = ((OdbcObject*)arg1)->sqlGetDiagRec(arg0, arg2, arg3, arg4, arg5, arg6, arg7);
    TRACERET("SQLGetDiagRec", retcode);
    return retcode;
}

///// SQLGetEnvAttr /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetEnvAttr(SQLHENV arg0,
                                            SQLINTEGER arg1,
                                            SQLPOINTER arg2,
                                            SQLINTEGER arg3,
                                            SQLINTEGER* arg4)
{
    TRACE("SQLGetEnvAttr");

    RETCODE retcode = ((OdbcEnv*)arg0)->sqlGetEnvAttr(arg1, arg2, arg3, arg4);
    TRACERET("SQLGetEnvAttr", retcode);
    return retcode;
}

///// SQLGetStmtAttr /////

RETCODE NUODB_ODBCAPI SQL_API SQLGetStmtAttr(SQLHSTMT arg0,
                                             SQLINTEGER arg1,
                                             SQLPOINTER arg2,
                                             SQLINTEGER arg3,
                                             SQLINTEGER* arg4)
{
    TRACE("SQLGetStmtAttr");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlGetStmtAttr(arg1, arg2, arg3, arg4);
    TRACERET("SQLGetStmtAttr", retcode);
    return retcode;
}

///// SQLSetConnectAttr /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetConnectAttr(SQLHDBC arg0,
                                                SQLINTEGER arg1,
                                                SQLPOINTER arg2,
                                                SQLINTEGER arg3)
{
    TRACE("SQLSetConnectAttr");

    RETCODE retcode = ((OdbcConnection*)arg0)->sqlSetConnectAttr(arg1, arg2, arg3);
    TRACERET("SQLSetConnectAttr", retcode);
    return retcode;
}

///// SQLSetDescField /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetDescField(SQLHDESC arg0,
                                              SQLSMALLINT arg1,
                                              SQLSMALLINT arg2,
                                              SQLPOINTER arg3,
                                              SQLINTEGER arg4)
{
    TRACE("SQLSetDescField");

    RETCODE retcode = ((OdbcDesc*)arg0)->sqlSetDescField(arg1, arg2, arg3, arg4);
    TRACERET("SQLSetDescField", retcode);
    return retcode;
}

///// SQLSetDescRec /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetDescRec(SQLHDESC arg0,
                                            SQLSMALLINT arg1,
                                            SQLSMALLINT arg2,
                                            SQLSMALLINT arg3,
                                            SQLLEN arg4,
                                            SQLSMALLINT arg5,
                                            SQLSMALLINT arg6,
                                            SQLPOINTER arg7,
                                            SQLLEN* arg8,
                                            SQLLEN* arg9)
{
    notYetImplemented("SQLSetDescRec called");
    return SQL_SUCCESS;
}

///// SQLSetEnvAttr /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetEnvAttr(SQLHENV arg0,
                                            SQLINTEGER arg1,
                                            SQLPOINTER arg2,
                                            SQLINTEGER arg3)
{
    TRACE("SQLSetEnvAttr");

    RETCODE retcode = ((OdbcEnv*)arg0)->sqlSetEnvAttr(arg1, arg2, arg3);
    TRACERET("SQLSetEnvAttr", retcode);
    return retcode;
}

///// SQLSetStmtAttr /////

RETCODE NUODB_ODBCAPI SQL_API SQLSetStmtAttr(SQLHSTMT arg0,
                                             SQLINTEGER arg1,
                                             SQLPOINTER arg2,
                                             SQLINTEGER arg3)
{
    TRACE("SQLSetStmtAttr");

    RETCODE retcode = ((OdbcStatement*)arg0)->sqlSetStmtAttr(arg1, arg2, arg3);
    TRACERET("SQLSetStmtAttr", retcode);
    return retcode;
}

///// SQLBulkOperations /////

RETCODE NUODB_ODBCAPI SQL_API SQLBulkOperations(SQLHSTMT arg0,
                                                SQLSMALLINT arg1)
{
    notYetImplemented("SQLBulkOperations called");
    return SQL_SUCCESS;
}
