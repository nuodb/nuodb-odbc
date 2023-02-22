/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "OdbcBase.h"
#include "OdbcConnection.h"
#include "ProductVersion.h"

#include <stdio.h>
#include <cstring>
#include <cassert>

#include <odbcinst.h>

#include "NuoRemote/CallableStatement.h"
#include "NuoRemote/Connection.h"
#include "NuoRemote/DatabaseMetaData.h"
#include "NuoRemote/PreparedStatement.h"
#include "NuoRemote/Properties.h"
#include "OdbcBase.h"
#include "OdbcEnv.h"
#include "SetupAttributes.h"
#include "SQLException.h"
#include "OdbcStatement.h"
#include "OdbcDesc.h"
#include "OdbcTrace.h"

using namespace NuoDB;

typedef Connection* (* ConnectFn)();

#define ODBC_DRIVER_VERSION "03.50"
// #define ODBC_DRIVER_VERSION   SQL_SPEC_STRING
#define ODBC_VERSION_NUMBER "03.50.0000"

static const int supportedFunctions[] = {
    // Deprecated but important stuff

    SQL_API_SQLALLOCCONNECT,
    SQL_API_SQLALLOCENV,
    SQL_API_SQLALLOCSTMT,
    SQL_API_SQLFREECONNECT,
    SQL_API_SQLFREEENV,
    SQL_API_SQLFREESTMT,
    SQL_API_SQLCOLATTRIBUTES,
    SQL_API_SQLERROR,
    SQL_API_SQLSETPARAM,
    SQL_API_SQLTRANSACT,

    SQL_API_SQLENDTRAN,

    SQL_API_SQLALLOCHANDLE,
    SQL_API_SQLGETDESCFIELD,
    SQL_API_SQLBINDCOL,
    SQL_API_SQLGETDESCREC,
    SQL_API_SQLCANCEL,
    SQL_API_SQLGETDIAGFIELD,
    SQL_API_SQLCLOSECURSOR,
    SQL_API_SQLGETDIAGREC,
    SQL_API_SQLCOLATTRIBUTE,
    SQL_API_SQLGETENVATTR,
    SQL_API_SQLCONNECT,
    SQL_API_SQLGETFUNCTIONS,
    SQL_API_SQLCOPYDESC,
    SQL_API_SQLGETINFO,
    SQL_API_SQLDATASOURCES,
    SQL_API_SQLGETSTMTATTR,
    SQL_API_SQLDESCRIBECOL,
    SQL_API_SQLGETTYPEINFO,
    SQL_API_SQLDISCONNECT,
    SQL_API_SQLNUMRESULTCOLS,
    SQL_API_SQLDRIVERS,
    SQL_API_SQLPARAMDATA,
    SQL_API_SQLENDTRAN,
    SQL_API_SQLPREPARE,
    SQL_API_SQLEXECDIRECT,
    SQL_API_SQLPUTDATA,
    SQL_API_SQLEXECUTE,
    SQL_API_SQLROWCOUNT,
    SQL_API_SQLFETCH,
    SQL_API_SQLSETCONNECTATTR,
    SQL_API_SQLFETCHSCROLL,
    SQL_API_SQLSETCURSORNAME,
    SQL_API_SQLFREEHANDLE,
    SQL_API_SQLSETDESCFIELD,
    SQL_API_SQLFREESTMT,
    SQL_API_SQLSETDESCREC,
    SQL_API_SQLGETCONNECTATTR,
    SQL_API_SQLSETENVATTR,
    SQL_API_SQLGETCURSORNAME,
    SQL_API_SQLSETSTMTATTR,
    SQL_API_SQLGETDATA,

    // The following is a list of valid values for FunctionId for functions conforming to the X/Open standards compliance level,,

    SQL_API_SQLCOLUMNS,
    SQL_API_SQLSTATISTICS,
    SQL_API_SQLSPECIALCOLUMNS,
    SQL_API_SQLTABLES,

    // The following is a list of valid values for FunctionId for functions conforming to the ODBC standards compliance level,,
    SQL_API_SQLBINDPARAMETER,
    SQL_API_SQLNATIVESQL,
    SQL_API_SQLBROWSECONNECT,
    SQL_API_SQLNUMPARAMS,
    SQL_API_SQLBULKOPERATIONS,
    SQL_API_SQLPRIMARYKEYS,
    SQL_API_SQLCOLUMNPRIVILEGES,
    SQL_API_SQLPROCEDURECOLUMNS,
    SQL_API_SQLDESCRIBEPARAM,
    SQL_API_SQLPROCEDURES,
    SQL_API_SQLDRIVERCONNECT,
    SQL_API_SQLSETPOS,
    SQL_API_SQLFOREIGNKEYS,
    SQL_API_SQLTABLEPRIVILEGES,
    SQL_API_SQLMORERESULTS,
};

enum InfoType { infoString, infoShort, infoLong, infoUnsupported };
struct TblInfoItem
{
    int item;
    const char* name;
    InfoType    type;
    const char* value;
};

#define MAX_SQLSYMBOL_LEN 128

#define CITEM(item, value)  {item, #item, infoString, value},
#define SITEM(item, value)  {item, #item, infoShort, (char*)value},
#define NITEM(item, value)  {item, #item, infoLong, (char*)value},
#define UITEM(item, value)  {item, #item, infoUnsupported, (char*)value},

static const TblInfoItem tblInfoItems[] = {
#include "InfoItems.h"
    {0, 0, infoShort, 0}
};

struct InfoItem
{
    const char* name;
    InfoType    type;
    const char* value;
};

#define INFO_SLOT(n) ((n < 10000) ? n : n - 10000 + 200)
#define INFO_SLOTS 400

static SQLUSMALLINT functionsArray[100];
static SQLSMALLINT functionsBitmap[SQL_API_ODBC3_ALL_FUNCTIONS_SIZE];
static int moduleInit();
static InfoItem infoItems[INFO_SLOTS];
static int foo = moduleInit();

/***
    #define SQL_FUNC_EXISTS(pfExists, uwAPI) \
    ((*(((UWORD*) (pfExists)) + ((uwAPI) >> 4)) \
    & (1 << ((uwAPI) & 0x000F)) \
    ) ? SQL_TRUE : SQL_FALSE \
***/

int moduleInit()
{
    // See documentation for SQLGetFunctions(). functionsArray needs to be an
    // array of 100 elements which represents the supported ODBC 2.x or earlier
    // functions, where SQL_TRUE means the function is supported. All pre-3.0
    // function ids are < 100.
    memset(functionsArray, SQL_FALSE, sizeof(functionsArray));
    for (unsigned int n = 0; n < sizeof(supportedFunctions)/sizeof(supportedFunctions[0]); ++n) {
        int fn = supportedFunctions[n];
        if (fn < 100) {
            functionsArray[fn] = SQL_TRUE;
        }
        assert((fn >> 4) < SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);
        functionsBitmap[fn >> 4] |= 1 << (fn & 0xf);
    }

    for (const TblInfoItem* t = tblInfoItems; t->name; ++t) {
        int slot = INFO_SLOT(t->item);
        assert(slot >= 0 && slot < INFO_SLOTS);
        InfoItem* item = infoItems + slot;
        assert(item->name == NULL);
        item->name = t->name;
        item->type = t->type;
        item->value = t->value;
    }

    DBG((">> moduleInit"));

    //bool test = SQL_FUNC_EXISTS(functionsBitmap, SQL_API_SQLALLOCHANDLE);

    return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OdbcConnection::OdbcConnection(OdbcEnv* parent)
{
    env = parent;
    connected = false;
    connectionTimeout = 0;
    connection = NULL;
    transactionPending = false;
    statements = NULL;
    descriptors = NULL;
    asyncEnabled = false;
    autoCommit = true;
    transactionIsolation = NuoDB::TRANSACTION_SERIALIZABLE;
    driver = DRIVER_FULL_NAME;
    DBG((">> OdbcConnection '%s'", driver.c_str()));
}

OdbcConnection::~OdbcConnection()
{
    close();
}

void OdbcConnection::close()
{
    if (env) {
        env->connectionClosed(this);
        env = NULL;
    }

    while (statements) {
        delete statements;
    }

    while (descriptors) {
        delete descriptors;
    }

    if (connection) {
        connection->close();
        connected = false;
        connection = NULL;
    }
}

OdbcObjectType OdbcConnection::getType()
{
    return odbcTypeConnection;
}

static std::string trim(const char* data, const char* trimChars)
{
    size_t dataLen = data ? strlen(data) : 0;
    if (dataLen == 0) {
        return std::string();
    }

    // trim left

    size_t trimStart = strspn(data, trimChars);

    // trim right

    size_t  len = dataLen - trimStart;
    bool    found = true;

    for (const char* p = data + dataLen - 1; found && len > 0; p--) {
        found = false;

        for (const char* t = trimChars; *t; t++) {
            if (*p == *t) {
                found = true;
                --len;

                break;
            }
        }
    }

    return std::string(data + trimStart, len);
}

RETCODE OdbcConnection::sqlBrowseConnect(SQLCHAR* inString,
                                         SQLSMALLINT inStringLen,
                                         SQLCHAR* outString,
                                         SQLSMALLINT outStringLen,
                                         SQLSMALLINT* outStringLenPtr)
{
    clearErrors();
    SQLLEN      length = stringLength(inString, inStringLen);
    const char* end = (const char*)inString + length;

    for (const char* p = (const char*)inString; p < end;) {
        char    name[256];
        char    value[256];
        char*   q = name;
        char    c = '\0';
        while (p < end && (c = *p++) != '=' && c != ';') {
            *q++ = c;
        }
        *q = 0;
        q = value;
        if (c == '=') {
            while (p < end && (c = *p++) != ';') {
                *q++ = c;
            }
        }
        *q = 0;
        if (!strcasecmp(name, "DSN")) {
            dsn = value;
        } else if (!strcasecmp(name, "DBNAME") || !strcasecmp(name, "DATABASE")) {
            databaseName = trim(value, "{}");
        } else if (!strcasecmp(name, "UID") || !strcasecmp(name, "UIC")) {
            account = value;
        } else if (!strcasecmp(name, "PWD")) {
            password = value;
        } else if (!strcasecmp(name, "DRIVER")) {
            driver = trim(value, "{}");
        } else if (!strcasecmp(name, "SCHEMA")) {
            schema = value;
        } else if (!strcasecmp(name, "ODBC")) {} else {
            std::ostringstream text;
            text << "Invalid connection string attribute: " << name;
            postError("01S00", text.str());
        }
    }

    char    returnString[1024], * r = returnString;
    bool    haveEverything = true;

    if (dsn.empty()) {
        haveEverything = false;
        r = appendAttribute("DSN", "?", r, r == returnString);
    }

    if (account.empty()) {
        haveEverything = false;
        r = appendAttribute("UIC", "?", r, r == returnString);
    }

    if (password.empty()) {
        haveEverything = false;
        r = appendAttribute("PWD", "?", r, r == returnString);
    }

    if (driver.empty()) {
        haveEverything = false;
        r = appendAttribute("DRIVER", "?", r, r == returnString);
    }

    if (schema.empty()) {
        r = appendAttribute("*SCHEMA", "?", r, r == returnString);
    }

    if (haveEverything) {

        // if we have everything then rebuild the full connect string so we can return it
        expandConnectParameters();

        r = returnString;
        r = appendAttribute("DSN", dsn.c_str(), r, r == returnString);
        r = appendAttribute("UIC", account.c_str(), r, r == returnString);
        r = appendAttribute("PWD", password.c_str(), r, r == returnString);
        r = appendAttribute("SCHEMA", schema.c_str(), r, r == returnString);
        r = appendAttribute("DRIVER", driver.c_str(), r, r == returnString); // last in the string to make Excel happier

        if (setString((UCHAR*)returnString, r - returnString, outString, outStringLen, outStringLenPtr)) {
            postError("01004", "String data, right truncated");
        }

        RETCODE ret = connect();

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            return ret;
        }

        return sqlSuccess();
    } else {
        if (setString((UCHAR*)returnString, r - returnString, outString, outStringLen, outStringLenPtr)) {
            postError("01004", "String data, right truncated");
        }

        return SQL_NEED_DATA;
    }
}

RETCODE OdbcConnection::sqlSetConnectAttr(SQLINTEGER arg1, SQLPOINTER arg2, SQLINTEGER stringLength)
{
    clearErrors();
    try {
        switch (arg1) {
            case SQL_ATTR_LOGIN_TIMEOUT:
                connectionTimeout = (SQLLEN)arg2;
                break;

            case SQL_ATTR_AUTOCOMMIT:
                autoCommit = (SQLLEN)arg2 == SQL_AUTOCOMMIT_ON;
                if (connection) {
                    connection->setAutoCommit(autoCommit);
                }
                break;

            case SQL_ATTR_TXN_ISOLATION: {
                if (transactionPending) {
                    return sqlReturn(SQL_ERROR, "25000", "Invalid  transaction state");
                }
                // The value is a 32bit bitmask.  Reduce SQLPOINTER to an int.
                // I'm not 100% sure this works, but I don't have a better idea.
                SQLLEN t = (SQLLEN)arg2;

                // ODBC isolation levels map exactly to our isolation
                // levels for example:
                //    SQL_TXN_READ_UNCOMMITTED == NuoDB::TRANSACTION_READ_UNCOMMITTED
                transactionIsolation = (int)t;
                if (connection) {
                    connection->setTransactionIsolation(transactionIsolation);
                }
                break;
            }
            case SQL_ATTR_ACCESS_MODE: {
                switch ((SQLLEN)arg2) {
                    case SQL_MODE_READ_ONLY:
                        connection->setReadOnly(true);
                        break;
                    case SQL_MODE_READ_WRITE:
                        connection->setReadOnly(false);
                        break;
                    default:
                        return sqlReturn(SQL_ERROR, "25000", "Invalid access mode");
                }
                break;
            }
        }
    } catch (SQLException& e) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(e.getSqlcode()), e);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcConnection::sqlDriverConnect(SQLHWND hWnd, const SQLCHAR* connectString,
                                         SQLSMALLINT connectStringLength,
                                         SQLCHAR* outConnectBuffer,
                                         SQLSMALLINT connectBufferLength,
                                         SQLSMALLINT* outStringLength,
                                         SQLUSMALLINT driverCompletion)
{
    clearErrors();

    if (connected) {
        return sqlReturn(SQL_ERROR, "08002", "Connection name is use");
    }

    if (connectStringLength < 0 && connectStringLength != SQL_NTS) {
        return sqlReturn(SQL_ERROR, "HY090", "Invalid string or buffer length");
    }

    switch (driverCompletion) {
        case SQL_DRIVER_COMPLETE:
        case SQL_DRIVER_COMPLETE_REQUIRED:
            break;

        case SQL_DRIVER_PROMPT:
            if (hWnd == NULL) {
                return sqlReturn(SQL_ERROR, "HY092", "Invalid attribute/option identifier");
            }
            break;
    }

    SQLLEN      length = stringLength(connectString, connectStringLength);
    const char* end = (const char*)connectString + length;

    for (const char* p = (const char*)connectString; p < end;) {
        char    name[256];
        char    value[256];
        char*   q = name;
        char    c = '\0';
        while (p < end && (c = *p++) != '=' && c != ';') {
            *q++ = c;
        }
        *q = 0;
        q = value;
        if (c == '=') {
            while (p < end && (c = *p++) != ';') {
                *q++ = c;
            }
        }
        *q = 0;
        if (!strcasecmp(name, "DSN")) {
            dsn = value;
        } else if (!strcasecmp(name, "DBNAME") || !strcasecmp(name, "DATABASE")) {
            databaseName = trim(value, "{}");
        } else if (!strcasecmp(name, "UID") || !strcasecmp(name, "UIC")) {
            account = value;
        } else if (!strcasecmp(name, "PWD")) {
            password = value;
        } else if (!strcasecmp(name, "DRIVER")) {
            driver = trim(value, "{}");
        } else if (!strcasecmp(name, "SCHEMA")) {
            schema = value;
        } else if (!strcasecmp(name, "ODBC")) {} else {
            std::ostringstream text;
            text << "Invalid connection string attribute: " << name;
            postError("01S00", text.str());
        }
    }

    expandConnectParameters();

    char returnString[1024], * r = returnString;

    r = appendAttribute("DSN", dsn.c_str(), r, r == returnString);
    r = appendAttribute("UIC", account.c_str(), r, r == returnString);
    r = appendAttribute("PWD", password.c_str(), r, r == returnString);
    r = appendAttribute("SCHEMA", schema.c_str(), r, r == returnString);
    r = appendAttribute("DRIVER", driver.c_str(), r, r == returnString); // last in the string to make Excel happier

    if (setString((UCHAR*)returnString, r - returnString, outConnectBuffer, connectBufferLength, outStringLength)) {
        postError("01004", "String data, right truncated");
    }

    RETCODE ret = connect();

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return ret;
    }

    return sqlSuccess();
}

std::string OdbcConnection::readAttribute(const char* attribute)
{
    char buffer[256];

    int ret = SQLGetPrivateProfileString(dsn.c_str(), attribute, "", buffer, sizeof(buffer), env->odbcIniFileName);

    return std::string(buffer, ret);
}

RETCODE OdbcConnection::sqlGetFunctions(SQLUSMALLINT functionId, SQLUSMALLINT* supportedPtr)
{
    clearErrors();

    switch (functionId) {
        case SQL_API_ODBC3_ALL_FUNCTIONS:
            memcpy(supportedPtr, functionsBitmap, sizeof(functionsBitmap));
            // memset (supportedPtr, -1, sizeof (functionsBitmap));
            return sqlSuccess();

        case SQL_API_ALL_FUNCTIONS:
            memcpy(supportedPtr, functionsArray, sizeof(functionsArray));
            return sqlSuccess();
    }

    /***
        if (functionId >= 0 && functionId < SQL_API_ODBC3_ALL_FUNCTIONS_SIZE  * 16)
        return sqlReturn (SQL_ERROR, "HY095", "Function type out of range");
    ***/

    *supportedPtr = (SQL_FUNC_EXISTS(functionsBitmap, functionId)) ? SQL_TRUE : SQL_FALSE;

    return sqlSuccess();
}

RETCODE OdbcConnection::sqlDisconnect()
{
    clearErrors();

    if (transactionPending) {
        return sqlReturn(SQL_ERROR, "25000", "Invalid transaction state: transaction pending");
    }

    try {

        if (connection) {
            connection->commit();
            close();
        }
    } catch (SQLException& exception) {
        postError("01002", exception);
        connection = NULL;
        connected = false;
        return SQL_SUCCESS_WITH_INFO;
    }

    return sqlSuccess();
}

RETCODE OdbcConnection::sqlGetInfo(SQLUSMALLINT type, SQLPOINTER ptr, SQLSMALLINT maxLength, SQLSMALLINT* actualLength)
{
    int slot = INFO_SLOT(type);
    if (slot < 0 || slot >= INFO_SLOTS) {
        std::ostringstream message;
        message << "Information type out of range: " << type;
        return sqlReturn(SQL_ERROR, "HY096", message.str().c_str());
    }

    InfoItem* item = infoItems + slot;
    if (item->name == NULL) {
        std::ostringstream message;
        message << "Information type out of range: " << type;
        return sqlReturn(SQL_ERROR, "HY096", message.str().c_str());
    }

    const char*         string = item->value;
    SQLUINTEGER         value = (SQLUINTEGER)(uintptr_t)item->value;
    DatabaseMetaData*   metaData = NULL;

    if (connection) {
        metaData = connection->getMetaData();
    } else {
        switch (type) {
            case SQL_ODBC_VER:
            case SQL_DRIVER_ODBC_VER:
            case SQL_ODBC_API_CONFORMANCE:
                break;

            default:
                return sqlReturn(SQL_ERROR, "08003", "Connection does not exist");
        }
    }
    try {
        switch (type) {
            case SQL_CORRELATION_NAME:
                string = "Y";
                break;

            case SQL_DYNAMIC_CURSOR_ATTRIBUTES1:
            case SQL_DYNAMIC_CURSOR_ATTRIBUTES2:
                break;

            case SQL_KEYSET_CURSOR_ATTRIBUTES1:
            case SQL_KEYSET_CURSOR_ATTRIBUTES2:
                value = 0;
                break;

            case SQL_STATIC_CURSOR_ATTRIBUTES1:
            case SQL_STATIC_CURSOR_ATTRIBUTES2:
                value = 2;
                break;

            case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1:
            case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2:
                value = 0;
                break;

            case SQL_NEED_LONG_DATA_LEN:
                string = "N";
                break;

            case SQL_MAX_CONCURRENT_ACTIVITIES:
                value = 0; // no limit
                break;

            case SQL_MAX_DRIVER_CONNECTIONS:
                value = 0; // no limit
                break;

            case SQL_CATALOG_LOCATION:
                value = 0; // catalogs not supported
                break;

            case SQL_NON_NULLABLE_COLUMNS:
                value = SQL_NNC_NON_NULL; // supports non-null columns
                break;

            case SQL_CONCAT_NULL_BEHAVIOR:
                value = SQL_CB_NULL; // null + non-null == null
                break;

            case SQL_DATA_SOURCE_NAME:
                string = dsn.c_str();
                break;

            case SQL_GROUP_BY:
                value = SQL_GB_GROUP_BY_EQUALS_SELECT;
                break;

            case SQL_IDENTIFIER_CASE:
                value = SQL_IC_UPPER;
                break;

            case SQL_MAX_INDEX_SIZE:
                value = 0; // no limit
                break;

            case SQL_MAX_ROW_SIZE:
                value = 0; // no limit
                break;

            case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
                string = "Y";
                break;

            case SQL_MAX_TABLES_IN_SELECT:
                value = 0; // no limit
                break;

            case SQL_NULL_COLLATION:
                value = SQL_NC_START;
                break;

            case SQL_ORDER_BY_COLUMNS_IN_SELECT:
                string = "Y";
                break;

            case SQL_QUOTED_IDENTIFIER_CASE:
                value = SQL_IC_MIXED;
                break;

            case SQL_INTEGRITY:
                string = "Y";
                break;

            case SQL_SUBQUERIES:
                value = SQL_SQ_CORRELATED_SUBQUERIES|SQL_SQ_COMPARISON|SQL_SQ_EXISTS|SQL_SQ_IN|SQL_SQ_QUANTIFIED;
                break;

            case SQL_MULT_RESULT_SETS:
                string = "N";
                break;

            case SQL_SERVER_NAME:
                string = dsn.c_str();
                break;

            case SQL_ACCESSIBLE_TABLES:
                string = "N";
                break;

            case SQL_MAX_COLUMNS_IN_GROUP_BY:
                value = 0; // no limit
                break;

            case SQL_MAX_COLUMNS_IN_INDEX:
                value = 0; // no limit
                break;

            case SQL_MAX_COLUMNS_IN_ORDER_BY:
                value = 0; // no limit
                break;

            case SQL_MAX_COLUMNS_IN_SELECT:
                value = 0; // no limit
                break;

            case SQL_MAX_COLUMNS_IN_TABLE:
                value = 0; // no limit
                break;

            case SQL_FILE_USAGE:
                value = SQL_FILE_NOT_SUPPORTED;
                break;

            case SQL_MAX_CATALOG_NAME_LEN:
                value = 255;
                break;

            case SQL_SCROLL_CONCURRENCY:
                value = SQL_SCCO_READ_ONLY;
                break;

            case SQL_LOCK_TYPES:
                value = 0; // aka not implemented
                break;

            case SQL_POS_OPERATIONS:
                value = 0; // aka not implemented
                break;

            case SQL_GETDATA_EXTENSIONS:
                value = 0; // aka not implemented
                break;

            case SQL_ODBC_VER:
                string = ODBC_VERSION_NUMBER;
                break;

            case SQL_DRIVER_ODBC_VER:
                string = ODBC_DRIVER_VERSION;
                break;

            case SQL_ODBC_API_CONFORMANCE:
                value = SQL_OAC_LEVEL2;
                break;

            case SQL_CURSOR_COMMIT_BEHAVIOR:
                if (metaData->supportsOpenCursorsAcrossCommit()) {
                    value = SQL_CB_PRESERVE;
                } else if (metaData->supportsOpenStatementsAcrossCommit()) {
                    value = SQL_CB_CLOSE;
                } else {
                    value = SQL_CB_DELETE;
                }
                break;

            case SQL_CURSOR_ROLLBACK_BEHAVIOR:
                if (metaData->supportsOpenCursorsAcrossRollback()) {
                    value = SQL_CB_PRESERVE;
                } else if (metaData->supportsOpenStatementsAcrossRollback()) {
                    value = SQL_CB_CLOSE;
                } else {
                    value = SQL_CB_DELETE;
                }
                break;

            case SQL_IDENTIFIER_QUOTE_CHAR:
                string = metaData->getIdentifierQuoteString();
                break;

            case SQL_DATABASE_NAME:
                string = databaseName.c_str();
                break;

            case SQL_DBMS_NAME:
                string = metaData->getDatabaseProductName();
                break;

            case SQL_DBMS_VER:
                string = metaData->getDatabaseProductVersion();
                break;

            case SQL_SCROLL_OPTIONS:
                value = SQL_SO_FORWARD_ONLY;
                break;

            case SQL_DEFAULT_TXN_ISOLATION:
                value = metaData->getDefaultTransactionIsolation();
                break;

            case SQL_TXN_ISOLATION_OPTION:
                value = getSupportedTransactionIsolationBitmask();
                break;

            case SQL_USER_NAME:
                string = metaData->getUserName();
                break;

            case SQL_DRIVER_NAME:
                string = DRIVER_NAME;
                break;

            case SQL_CATALOG_NAME:
                string = "N";
                break;

            case SQL_CATALOG_USAGE:
                value = 0;
                break;

            case SQL_CATALOG_TERM:
                string = metaData->getCatalogTerm();
                break;

            case SQL_CATALOG_NAME_SEPARATOR:
                string = ".";
                break;

            case SQL_SCHEMA_TERM:
                string = metaData->getSchemaTerm();
                break;

            case SQL_SCHEMA_USAGE:
                value = SQL_SU_DML_STATEMENTS;
                value |= SQL_SU_PROCEDURE_INVOCATION;
                value |= SQL_SU_TABLE_DEFINITION;
                value |= SQL_SU_INDEX_DEFINITION;
                value |= SQL_SU_PRIVILEGE_DEFINITION;
                break;

            case SQL_DRIVER_VER:
                string = NUOODBC_VERSION_STR;
                break;

            case SQL_TABLE_TERM:
                string = "table";
                break;

            case SQL_PROCEDURES:
            case SQL_ACCESSIBLE_PROCEDURES:
                string = "Y";
                break;

            case SQL_PROCEDURE_TERM:
                string = metaData->getProcedureTerm();
                break;

            case SQL_KEYWORDS:
                string = "ABS, ACOS, ADD_PARTITION, ASIN, ATAN2, ATAN, BITS, BIT_LENGTH, BREAK, "
                        "CASCADE, CATCH, CEILING, CHARACTER_LENGTH, COALESCE, CONCAT, CONTAINING, "
                        "CONVERT_TZ, COS, COT, CURRENT_SCHEMA, DATE_ADD, DATE_SUB, DAYOFWEEK, "
                        "DAYOFYEAR, DEGREES, DROP_PARTITION, END_FOR, END_FUNCTION, END_IF, "
                        "END_PROCEDURE, END_TRIGGER, END_TRY, END_WHILE, ENUM, EXTRACT, FLOOR, "
                        "FOR_UPDATE, GENERATED, IF, IFNULL, KEY, LAST_INSERT_ID, LIMIT, LOCATE, "
                        "LOWER, LTRIM, MAXVALUE, MOD, MSLEEP, NEXT, NEXT_VALUE, NOW, NULLIF, "
                        "NVARCHAR, OCTETS, OCTET_LENGTH, OFF, OFFSET, PI, POSITION, POWER, RADIANS, "
                        "RAND, RECORD_BATCHING, REGEXP, RESTART, RESTRICT, REVERSE, ROUND, RTRIM, "
                        "SHOW, SIN, SQRT, STARTING, STORE IN, STRING_TYPE, SUBSTRING_INDEX, "
                        "SUBSTR, TAN, THROW, TRIM, TRY, VAR, VER, WHILE";
                break;
            case SQL_NUMERIC_FUNCTIONS:
                string = "abs,acos,asin,atan,atan2,ceiling,cos,cot,degrees,floor,mod,pi,power,radians,rand,round,sin,sqrt,tan";
                break;
            case SQL_STRING_FUNCTIONS:
                string = "concat,lcase,left,length,locate,ltrim,replace,right,rtrim,substring,ucase";
                break;
            case SQL_SYSTEM_FUNCTIONS:
                string = "database,ifnull,user";
                break;
            case SQL_TIMEDATE_FUNCTIONS:
                string = "year,month,day,hour,minute,second,dayofweek,dayofyear,extract,"
                        "current_date,current_time,current_timestamp,now,convert_tz,date,"
                        "date_add,date_sub,date_from_str,date_to_str";
                break;
            case SQL_SEARCH_PATTERN_ESCAPE:
                string = metaData->getSearchStringEscape();
                break;

            default:
                TRACE(formatString("SQLGetInfo: Unknown getInfo type %u", type).c_str());
        }
    } catch (SQLException& e) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(e.getSqlcode()), e);
        return SQL_ERROR;
    }
    switch (item->type) {
        case infoString:
#ifdef DEBUG
            {
                OutputDebugString(formatString("  %s (string) %s\n", item->name, string).c_str());
            }
#endif
            return (setString(string, (SQLCHAR*)ptr, maxLength, actualLength))
                   ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;

        case infoShort:
#ifdef DEBUG
            {
                OutputDebugString(formatString("  %s (short) %u\n", item->name, value).c_str());
            }
#endif
            if (ptr) {
                *((SQLUSMALLINT*)ptr) = (SQLUSMALLINT)value;
            }
            if (actualLength) {
                *actualLength = sizeof(SQLUSMALLINT);
            }
            break;

        case infoLong:
#ifdef DEBUG
            {
                OutputDebugString(formatString("  %s (long) %u\n", item->name, value).c_str());
            }
#endif
            if (ptr) {
                *((SQLUINTEGER*)ptr) = value;
            }
            if (actualLength) {
                *actualLength = sizeof(SQLUINTEGER);
            }
            break;

        case infoUnsupported:
#ifdef DEBUG
            {
                OutputDebugString(formatString("  %s (string) %s\n", item->name, "*unsupported*").c_str());
            }
#endif
            if (ptr) {
                *((SQLUINTEGER*)ptr) = value;
            }
            break;
    }

    return sqlSuccess();
}

char* OdbcConnection::appendAttribute(const char* name, const char* value, char* ptr, bool empty)
{
    if (!value) {
        return ptr;
    }

    if (!empty) {
        ptr = appendString(ptr, ";");
    }

    ptr = appendString(ptr, name);
    ptr = appendString(ptr, "=");
    return appendString(ptr, value);
}

char* OdbcConnection::appendString(char* ptr, const char* string)
{
    while (*string) {
        *ptr++ = *string++;
    }

    return ptr;
}

RETCODE OdbcConnection::allocHandle(int handleType, SQLHANDLE* outputHandle)
{
    clearErrors();
    *outputHandle = SQL_NULL_HDBC;

    if (handleType != SQL_HANDLE_STMT) {
        return sqlReturn(SQL_ERROR, "HY000", "General Error");
    }

    OdbcStatement* statement = new OdbcStatement(this);
    statement->next = statements;
    statements = statement;
    *outputHandle = statement;

    return sqlSuccess();
}

DatabaseMetaData* OdbcConnection::getMetaData()
{
    return connection->getMetaData();
}

RETCODE OdbcConnection::sqlConnect(const SQLCHAR* dataSetName, SQLSMALLINT dsnLength, SQLCHAR* uid, SQLSMALLINT uidLength, SQLCHAR* passwd, SQLSMALLINT passwdLength)
{
    clearErrors();

    if (connected) {
        return sqlReturn(SQL_ERROR, "08002", "Connection name is use");
    }

    char temp[1024], * p = temp;

    dsn = getString(&p, dataSetName, dsnLength, "");
    account = getString(&p, uid, uidLength, "");
    password = getString(&p, passwd, passwdLength, "");
    expandConnectParameters();
    RETCODE ret = connect();

    if (ret != SQL_SUCCESS) {
        return ret;
    }

    return sqlSuccess();
}

RETCODE OdbcConnection::connect()
{
    Properties* properties = NULL;

    DBG((">> connect"));

    try {
        connection = NuoDB_createConnection();
        properties = connection->allocProperties();
        if (!account.empty()) {
            properties->putValue("user", account.c_str());
        }
        if (!password.empty()) {
            properties->putValue("password", password.c_str());
        }
        if (!schema.empty()) {
            properties->putValue("schema", schema.c_str());
        }

        DBG((">> openDatabase %s: user=%s schema=%s", databaseName.c_str(),
             account.c_str(), schema.c_str()));

        connection->openDatabase(databaseName.c_str(), properties);
        properties->release();
    } catch (SQLException& exception) {

        if (properties) {
            properties->release();
        }

        std::string text = exception.getText();
        connection->close();
        connection = NULL;

        return sqlReturn(SQL_ERROR, "08004", text.c_str());
    }

    connection->setTransactionIsolation(transactionIsolation);
    connected = true;

    return SQL_SUCCESS;
}

RETCODE OdbcConnection::sqlEndTran(SQLUSMALLINT operation)
{
    clearErrors();

    if (connection) {
        try {
            switch (operation) {
                case SQL_COMMIT:
                    connection->commit();
                    break;

                case SQL_ROLLBACK:
                    connection->rollback();
                    break;
            }
            transactionPending = false;
        } catch (SQLException& exception) {
            postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
            return SQL_ERROR;
        }
    }

    return sqlSuccess();
}

void OdbcConnection::statementDeleted(OdbcStatement* statement)
{
    for (OdbcObject** ptr = (OdbcObject**)&statements; *ptr; ptr = &((*ptr)->next)) {
        if (*ptr == statement) {
            *ptr = statement->next;
            break;
        }
    }
}

void OdbcConnection::expandConnectParameters()
{
    if (!dsn.empty()) {
        if (databaseName.empty()) {
            databaseName = readAttribute(SETUP_DBNAME);
        }

        // If we still don't have a database name, attempt to construct it
        // from commonly-used attributes
        if (databaseName.empty()) {
            databaseName = readAttribute(SETUP_DATABASE);
            if (! databaseName.empty()) {
                std::string server = readAttribute(SETUP_SERVERNAME);
                if (! server.empty()) {
                    databaseName += "@";
                    databaseName += server;

                    std::string port = readAttribute(SETUP_PORT);
                    if (! port.empty()) {
                        databaseName += ":";
                        databaseName += port;
                    }
                }
            }
        }

        if (account.empty()) {
            account = readAttribute(SETUP_USER);
        }

        if (password.empty()) {
            password = readAttribute(SETUP_PASSWORD);
        }

        if (schema.empty()) {
            schema = readAttribute(SETUP_SCHEMA);
        }
    }
}

OdbcDesc* OdbcConnection::allocDescriptor(OdbcDescType type)
{
    OdbcDesc* descriptor = new OdbcDesc(type, this);
    descriptor->next = descriptors;
    descriptors = descriptor;

    return descriptor;
}

void OdbcConnection::descriptorDeleted(OdbcDesc* descriptor)
{
    for (OdbcDesc** ptr = &descriptors; *ptr; ptr = (OdbcDesc**)&(*ptr)->next) {
        if (*ptr == descriptor) {
            *ptr = (OdbcDesc*)descriptor->next;
            break;
        }
    }
}

RETCODE OdbcConnection::sqlGetConnectAttr(SQLINTEGER attribute, SQLPOINTER ptr, SQLINTEGER bufferLength, SQLINTEGER* lengthPtr)
{
    clearErrors();
    long        value;
    const char* string = NULL;
    bool        stringValue = false; // flag for saying that this is a string value

    switch (attribute) {
        case SQL_ATTR_ASYNC_ENABLE:
            value = asyncEnabled;
            break;

        case SQL_ACCESS_MODE:   //   101
            value = connection->getMetaData()->isReadOnly() ? SQL_MODE_READ_ONLY : SQL_MODE_READ_WRITE;
            break;

        case SQL_TXN_ISOLATION: //   108
            value = connection->getTransactionIsolation();
            break;

        case SQL_AUTOCOMMIT:    //   102
            value = (autoCommit) ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
            break;

        case SQL_LOGIN_TIMEOUT: //   103
        case SQL_OPT_TRACE: //   104
        case SQL_OPT_TRACEFILE: //   105
        case SQL_TRANSLATE_DLL: //   106
        case SQL_TRANSLATE_OPTION:  // 107
        case SQL_CURRENT_QUALIFIER: // 109
        case SQL_ODBC_CURSORS:  //   110
        case SQL_QUIET_MODE:    //   111
        case SQL_PACKET_SIZE:   //   112

        default:
            return sqlReturn(SQL_ERROR, "HYC00", "Optional feature not implemented");
    }

    if (stringValue) {
        return returnStringInfo(ptr, bufferLength, lengthPtr, string);
    }

    if (ptr) {
        *(long*)ptr = value;
    }

    if (lengthPtr) {
        *lengthPtr = sizeof(long);
    }

    return sqlSuccess();
}

void OdbcConnection::transactionStarted()
{
    if (!autoCommit) {
        transactionPending = true;
    }
}

int32_t OdbcConnection::getSupportedTransactionIsolationBitmask()
{
    int32_t result = 0;

    if (connection->getMetaData()->supportsTransactionIsolationLevel(NuoDB::TRANSACTION_READ_UNCOMMITTED)) {
        result |= SQL_TXN_READ_UNCOMMITTED;
    }

    if (connection->getMetaData()->supportsTransactionIsolationLevel(NuoDB::TRANSACTION_READ_COMMITTED)) {
        result |= SQL_TXN_READ_COMMITTED;
    }

    if (connection->getMetaData()->supportsTransactionIsolationLevel(NuoDB::TRANSACTION_REPEATABLE_READ)) {
        result |= SQL_TXN_REPEATABLE_READ;
    }

    if (connection->getMetaData()->supportsTransactionIsolationLevel(NuoDB::TRANSACTION_SERIALIZABLE)) {
        result |= SQL_TXN_SERIALIZABLE;
    }

    return result;
}

CallableStatement* OdbcConnection::prepareCall(const char* sql)
{
    return connection->prepareCall(sql);
}

PreparedStatement* OdbcConnection::prepareStatement(const char* sql)
{
    return connection->prepareStatement(sql, NuoDB::RETURN_GENERATED_KEYS);
}
