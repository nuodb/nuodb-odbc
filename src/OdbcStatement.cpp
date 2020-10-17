/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

// std::wstring_convert is deprecated in C++17, but there's no standard
// replacement yet, so keep using it.  An alternative would be to switch
// to ICU or similar, but...
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <string>

// See issue above related to C++17 deprecation.
#include <locale>
#include <codecvt>
#include <algorithm>

#include "OdbcStatement.h"

#include "OdbcBase.h"
#include "GetDataTypeFilter.h"
#include "OdbcConnection.h"
#include "OdbcError.h"
#include "OdbcTrace.h"
#include "OdbcTypeMapper.h"
#include "ResultSetMapper.h"

#include "NuoRemote/Blob.h"
#include "NuoRemote/CallableStatement.h"
#include "NuoRemote/DateClass.h"
#include "NuoRemote/DatabaseMetaData.h"
#include "NuoRemote/ParameterMetaData.h"
#include "NuoRemote/PreparedStatement.h"
#include "NuoRemote/ResultSet.h"
#include "NuoRemote/ResultSetMetaData.h"
#include "NuoRemote/TimeClass.h"
#include "NuoRemote/Timestamp.h"
#include "SQLException.h"

using namespace NuoDB;

/* Default to the POSIX API; make Windows groks it. */
#ifdef _WIN32
# define localtime_r(_t, _r) localtime_s(_r, _t)
#endif

#ifdef _WIN32
# define widechar_t wchar_t
# define widestring std::wstring
#else
# define widechar_t char16_t
# define widestring std::u16string
#endif

#define RESULTS(fn)         (resultSet ? resultSet->fn : callableStatement->fn)
#define SKIP_WHITE(p)       while (ODBC_STATEMENT::charTable[(int)*(p)] == WHITE) ++(p)

#define PUNCT   1
#define WHITE   2
#define DIGIT   4
#define LETTER  8
#define IDENT   (LETTER | DIGIT)
// An arbitrarily high number for binding columns when
// metadta is not available
#define MAX_COLUMN_NUM_SQLBIND_WITHOUT_METADATA 10000

namespace ODBC_STATEMENT {

static char charTable[256];
static int init();
static int foo = init();

int init()
{
    int         n;
    const char* p;

    for (p = " \t\n"; *p; ++p) {
        charTable[int(*p)] = WHITE;
    }

    for (p = "?=(),{}"; *p; ++p) {
        charTable[int(*p)] = PUNCT;
    }

    for (n = 'a'; n <= 'z'; ++n) {
        charTable[n] |= LETTER;
    }

    for (n = 'A'; n <= 'Z'; ++n) {
        charTable[n] |= LETTER;
    }

    for (n = '0'; n <= '9'; ++n) {
        charTable[n] |= DIGIT;
    }

    return 0;
}

} // namespace ODBC_STATEMENT

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OdbcStatement::OdbcStatement(OdbcConnection* connect)
    : connection(connect),
      applicationRowDescriptor(connect->allocDescriptor(odtApplicationRow)),
      applicationParamDescriptor(connect->allocDescriptor(odtApplicationParameter)),
      implementationRowDescriptor(connect->allocDescriptor(odtImplementationRow)),
      implementationParamDescriptor(connect->allocDescriptor(odtImplementationParameter))
{
}

OdbcStatement::~OdbcStatement()
{
    connection->statementDeleted(this);
    releaseResultSet();
    releaseStatement();
    fetchBindings.release();
    parameters.release();
    getDataBindings.release();
}

OdbcObjectType OdbcStatement::getType()
{
    return odbcTypeStatement;
}

RETCODE OdbcStatement::sqlTables(SQLCHAR* catalog, SQLSMALLINT catLength,
                                 SQLCHAR* schema, SQLSMALLINT schemaLength,
                                 SQLCHAR* table, SQLSMALLINT tableLength,
                                 SQLCHAR* type, SQLSMALLINT typeLength)
{
    clearErrors();
    releaseStatement();
    char temp[1024], * p = temp;
    sqlStmt = "sqlTables";

    const char* cat = getString(&p, catalog, catLength, NULL);
    const char* scheme = getString(&p, schema, schemaLength, NULL);
    const char* tbl = getString(&p, table, tableLength, NULL);
    const char* typeString = getString(&p, type, typeLength, "");

    const char* typeVector[16];
    int         numberTypes = 0;

    for (const char* q = typeString; *q && numberTypes < 16; ++numberTypes) {
        typeVector[numberTypes] = p;
        for (char c; *q && (c = *q++) != ',';) {
            *p++ = c;
        }

        *p++ = 0;
    }

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        setResultSet(dbMetaData->getTables(cat, scheme, tbl, numberTypes, typeVector));
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlPrepare(SQLCHAR* sql, SQLINTEGER sqlLength)
{
    clearErrors();
    releaseStatement();
    std::string temp;
    const char* string = (const char*)sql;
    if (sqlLength != SQL_NTS) {
        temp.assign((const char*)sql, sqlLength);
        string = temp.c_str();
    }

    sqlStmt = string;
#ifdef DEBUG
    TRACE(string);
#endif

    try {
        if (isStoredProcedureEscape(string)) {
            callableStatement = connection->prepareCall(string);
            statement = callableStatement;
        } else {
            statement = connection->prepareStatement(string);
            //PreparedStatement owns the ResultSetMetaData object
            metaData = statement->getMetaData();
            if (metaData) {
                numberColumns = metaData->getColumnCount();
            }
        }
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

void OdbcStatement::releaseStatement()
{
    releaseResultSet();
    callableStatement = NULL;
    metaData = nullptr;
    numberColumns = 0;

    if (statement) {
        statement->close();
        statement = NULL;
    }
}

void OdbcStatement::releaseResultSet()
{
    if (resultSet) {
        resultSet = NULL;
        metaData = NULL;
    }
    getDataBindings.release();
}

void OdbcStatement::setResultSet(ResultSet* results)
{
    releaseResultSet();

    resultSet = results;
    metaData = resultSet->getMetaData();
    numberColumns = metaData->getColumnCount();
    eof = false;
    cancel = false;
}

RETCODE OdbcStatement::sqlBindCol(SQLUSMALLINT column, SQLSMALLINT targetType, SQLPOINTER targetValuePtr, SQLLEN bufferLength, SQLLEN* indPtr)
{
    clearErrors();

    if (metaData) {
        TRACE(formatString("SQLBindCol: col %u type %d bufferlen " SQLLEN_FMT, column, targetType, bufferLength).c_str());
        if (column > numberColumns) {
            return sqlReturn(SQL_ERROR, "07009", "Invalid descriptor index");
        }
        fetchBindings.alloc(numberColumns + 1);
    } else if (column > MAX_COLUMN_NUM_SQLBIND_WITHOUT_METADATA) {
        return sqlReturn(SQL_ERROR, "07009", "Invalid descriptor index");
    } else if (column + 1 > fetchBindings.getCount()) {
        TRACE(formatString("SQLBindCol: col %u type %d bufferlen " SQLLEN_FMT, column, targetType, bufferLength).c_str());
        fetchBindings.alloc(column + 1);
    }

    switch (targetType) {
        case SQL_C_CHAR:
        case SQL_C_WCHAR:
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
        case SQL_C_SHORT:
        case SQL_C_SLONG:
        case SQL_C_ULONG:
        case SQL_C_LONG:
        case SQL_C_FLOAT:
        case SQL_C_DOUBLE:
        case SQL_C_BIT:
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
        case SQL_C_TINYINT:
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
        case SQL_C_BINARY:
        case SQL_C_DATE:
        case SQL_C_TIME:
        case SQL_C_TIMESTAMP:
        case SQL_C_NUMERIC:
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
        case SQL_LONGVARCHAR:
        case SQL_C_DEFAULT:
            break;

        default:
            std::ostringstream msg;
            msg << "Invalid application buffer type " << targetType;
            return sqlReturn(SQL_ERROR, "HY03", msg.str().c_str());
    }

    Binding* binding = fetchBindings.getBinding(column);
    binding->type = SQL_PARAM_OUTPUT;
    binding->cType = targetType;
    binding->pointer = targetValuePtr;
    binding->bufferLength = bufferLength;
    binding->indicatorPointer = indPtr;

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlFetch()
{
    TRACE(formatString("SQLFetch on: %s", sqlStmt.c_str()).c_str());
    if (!resultSet) {
        if (infoPosted) {
            return sqlSuccess();
        } else {
            return sqlReturn(SQL_ERROR, "24000", "Invalid cursor state");
        }
    }

    if (cancel) {
        releaseResultSet();
        return sqlReturn(SQL_ERROR, "S1008", "Operation canceled");
    }

    if (rowStatusPtr) {
        for (SQLULEN row = 0; row < rowArraySize; row++) {
            rowStatusPtr[row] = SQL_NO_DATA;
        }
    }

    rowCountPerFetch = 0;

    for (SQLULEN row = 0; row < rowArraySize; row++) {

        try {

            if (eof || (maxRowsPerSelect > 0 && rowCountPerSelect >= maxRowsPerSelect) || !resultSet->next()) {
                eof = true;
                TRACE("No more data");
                return rowCountPerFetch > 0 ? sqlSuccess() : SQL_NO_DATA;
            } else {
                TRACE(formatString("more data on row " SQLULEN_FMT, rowCountPerFetch).c_str());
            }
        } catch (SQLException& exception) {
            postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
            return SQL_ERROR;
        }

        fetchBindings.reset();

        for (int n = 1; n <= fetchBindings.getCount(); ++n) {
            Binding* binding = fetchBindings.getBinding(n);

            if (binding->pointer && binding->type != SQL_PARAM_INPUT) {
                if (setValue(binding, n, false, rowCountPerFetch, rowSize) == SQL_ERROR) {
                    return SQL_ERROR;
                }
            }
        }

        getDataBindings.reset();

        if (rowStatusPtr) {
            rowStatusPtr[rowCountPerFetch] = SQL_ROW_SUCCESS;
        }

        rowCountPerFetch++;
        rowCountPerSelect++;

        if (rowCountPerFetchPtr) {
            *rowCountPerFetchPtr = rowCountPerFetch;
        }
    }

    return sqlSuccess();
}

int OdbcStatement::setValue(Binding* binding, int column, bool indicatorIsRemaining, SQLULEN rowIndex, SQLULEN rowSize)
{
    TRACE(formatString("setValue on '%s' column %d type %d buflen " SQLLEN_FMT " offset " SQLLEN_FMT " rowIndex " SQLULEN_FMT " rowSize " SQLULEN_FMT, sqlStmt.c_str(), column, binding->cType, binding->bufferLength, binding->offset, rowIndex, rowSize).c_str());
    SQLLEN  bufferLength = binding->bufferLength;
    int     exitCode = SQL_SUCCESS;
    int     cType;
    PTR     bufferPtr;
    PTR     indicatorPtr;
    SQLLEN  remainingBytes = 0;

    if (rowSize == SQL_BIND_BY_COLUMN) {
        bufferPtr = binding->pointer;
        indicatorPtr = (char*)binding->indicatorPointer + (rowIndex * sizeof(SQLLEN));
        // rowIndex is still interesting in this case.  We need to calc the offset on a pertype basis
    } else {
        bufferPtr = (char*)binding->pointer + (rowIndex*rowSize);
        indicatorPtr = binding->indicatorPointer ? ((char*)binding->indicatorPointer + (rowIndex*rowSize)) : 0;
        // we no longer care about the rowIndex in this function, since we have just calculated the offset
        rowIndex = 0;
    }

    switch (binding->cType) {
        case SQL_C_DEFAULT:
            cType = convertFromSQL_C_DEFAULT((int)OdbcTypeMapper::mapType(resultSet->getMetaData()->getColumnType(column)));
            break;

        default:
            cType = binding->cType;
            break;
    }

    try {
        switch (cType) {
            case SQL_C_CHAR: {
                if (bufferLength < 0) {
                    return sqlReturn(SQL_ERROR, "HY090", "Invalid string or buffer length");
                }

                bufferPtr = (char*)bufferPtr + (bufferLength * rowIndex);
                const char* string = RESULTS(getString(column));
                SQLLEN stringLen = string == NULL ? 0 : strlen(string);
                SQLLEN valueLength = stringLen - binding->offset;
                SQLLEN maxlen = std::min<SQLLEN>(bufferLength-1, valueLength);

                if (valueLength <= 0 || maxlen < 0) {
                    maxlen = 0;
                } else {
                    memcpy(bufferPtr, string + binding->offset, maxlen);

                    if (maxlen != valueLength) {
                        exitCode = SQL_SUCCESS_WITH_INFO;
                        std::ostringstream msg;
                        msg << "Data truncated on column " << column << ", need length " << valueLength << " only have " << maxlen;
                        postError("01004", msg.str());
                    }
                }

                remainingBytes = stringLen - binding->offset;

                // if we have reached the end and this is a single call then reset

                if (binding->offset + maxlen > stringLen && binding->count == 0) { // reached the end
                    binding->offset = 0;
                } else {
                    binding->offset += maxlen;
                }

                // if we have actually populated the buffer or if this is the first
                // time through then NULL terminate the buffer

                if (maxlen > 0 || binding->count == 0) {
                    ((char*)(bufferPtr))[maxlen] = 0;   // always null terminated
                }
                bufferLength = maxlen;

                break;
            }

            case SQL_C_WCHAR: {
                if (bufferLength < 0) {
                    return sqlReturn(SQL_ERROR, "HY090", "Invalid string or buffer length");
                }

                bufferPtr = (char*)bufferPtr + (bufferLength * rowIndex);
                widestring wString = std::wstring_convert<std::codecvt_utf8_utf16<widechar_t>, widechar_t>{}.from_bytes(RESULTS(getString(column)));
                SQLLEN stringLen = 2* wString.length(); // Number of bytes to copy = 2 * length of wide chars string
                SQLLEN valueLength = stringLen - binding->offset;
                SQLLEN maxlen = std::min<SQLLEN>(bufferLength-2, valueLength); // 2 byes for a null terminated wide char string

                if (valueLength <= 0 || maxlen < 0) {
                    maxlen = 0;
                } else {
                    memcpy((char*)bufferPtr, (char*)wString.data() + binding->offset, maxlen);

                    if (maxlen != valueLength) {
                        exitCode = SQL_SUCCESS_WITH_INFO;
                        std::ostringstream msg;
                        msg << "Data truncated on column " << column << ", need length " << valueLength << " only have " << maxlen;
                        postError("01004", msg.str());
                    }
                }

                remainingBytes = stringLen - binding->offset;

                // if we have reached the end and this is a single call then reset

                if (binding->offset + maxlen > stringLen && binding->count == 0) { // reached the end
                    binding->offset = 0;
                } else {
                    binding->offset += maxlen;
                }

                // if we have actually populated the buffer or if this is the first
                // time through then NULL terminate the buffer

                if (maxlen > 0 || binding->count == 0) {
                    ((char16_t*)(bufferPtr))[maxlen/2] = 0;   // always null terminated
                }
                bufferLength = maxlen;

                break;

            }

            case SQL_C_BINARY: {
                if (bufferLength < 0) {
                    return sqlReturn(SQL_ERROR, "HY090", "Invalid string or buffer length");
                }

                bufferPtr = (char*)bufferPtr + (bufferLength * rowIndex);
                NuoDB::Blob* blob = RESULTS(getBlob(column));

                remainingBytes = blob->length() - binding->offset;
                SQLLEN maxlen = std::min<SQLLEN>(bufferLength, remainingBytes);

                if (remainingBytes <= 0 || maxlen < 0) {
                    maxlen = 0;
                } else {
                    blob->getBytes((int)binding->offset, (int)maxlen, (unsigned char*)bufferPtr);

                    if (maxlen != remainingBytes) {
                        exitCode = SQL_SUCCESS_WITH_INFO;
                        std::ostringstream msg;
                        msg << "Data truncated on column " << column << ", need length " << remainingBytes << " only have " << maxlen;
                        postError("01004", msg.str());
                    }
                }

                // if we have reached the end and this is a single call then reset

                if (binding->offset + maxlen > blob->length() && binding->count == 0) { // reached the end
                    binding->offset = 0;
                } else {
                    binding->offset += maxlen;
                }

                bufferLength = maxlen;
                break;
            }

            case SQL_C_SSHORT:
            case SQL_C_USHORT:
            case SQL_C_SHORT:
                bufferPtr = (char*)bufferPtr + (sizeof(short) * rowIndex);
                *((short*)bufferPtr) = RESULTS(getShort(column));
                remainingBytes = bufferLength = sizeof(short);
                break;

            // These three map to SQLINTEGER which is a 32 bit int
            // so use SQLINTEGER here so that we don't use 8 bytes
            // on 64 bit unix boxes where long is 8 bytes.
            case SQL_C_SLONG:
            case SQL_C_ULONG:
            case SQL_C_LONG:
                bufferPtr = (char*)bufferPtr + (sizeof(int) * rowIndex);
                *((int*)bufferPtr) = RESULTS(getInt(column));
                remainingBytes = bufferLength = sizeof(int);
                break;

            case SQL_C_FLOAT:
                bufferPtr = (char*)bufferPtr + (sizeof(float) * rowIndex);
                *((float*)bufferPtr) = RESULTS(getFloat(column));
                remainingBytes = bufferLength = sizeof(float);
                break;

            case SQL_C_DOUBLE:
                bufferPtr = (char*)bufferPtr + (sizeof(double) * rowIndex);
                *((double*)bufferPtr) = RESULTS(getDouble(column));
                remainingBytes = bufferLength = sizeof(double);
                break;

            case SQL_C_STINYINT:
            case SQL_C_UTINYINT:
            case SQL_C_TINYINT:
                bufferPtr = (char*)bufferPtr + (sizeof(char) * rowIndex);
                *((char*)bufferPtr) = RESULTS(getByte(column));
                remainingBytes = bufferLength = sizeof(char);
                break;

            case SQL_C_SBIGINT:
            case SQL_C_UBIGINT:
                bufferPtr = (char*)bufferPtr + (sizeof(int64_t) * rowIndex);
                *((int64_t*)bufferPtr) = RESULTS(getLong(column));
                remainingBytes = bufferLength = sizeof(int64_t);
                break;

            case SQL_TYPE_DATE:
            case SQL_C_DATE: {
                bufferPtr = (char*)bufferPtr + (sizeof(tagDATE_STRUCT) * rowIndex);
                NuoDB::Date*    date = RESULTS(getDate(column));
                time_t          time_secs = date->getSeconds();
                date->release();

                struct tm local_time;
                localtime_r(&time_secs, &local_time);

                tagDATE_STRUCT* var = (tagDATE_STRUCT*)bufferPtr;
                var->year = local_time.tm_year + 1900;
                var->month = local_time.tm_mon + 1;
                var->day = local_time.tm_mday;

                remainingBytes = bufferLength = sizeof(tagDATE_STRUCT);
                break;
            }

            case SQL_TYPE_TIMESTAMP:
            case SQL_C_TIMESTAMP: {
                bufferPtr = (char*)bufferPtr + (sizeof(tagTIMESTAMP_STRUCT) * rowIndex);
                NuoDB::Timestamp*       timestamp = RESULTS(getTimestamp(column));
                time_t                  time_secs = timestamp->getSeconds();
                tagTIMESTAMP_STRUCT*    var = (tagTIMESTAMP_STRUCT*)bufferPtr;
                var->fraction = timestamp->getNanos() * 10;
                timestamp->release();

                struct tm local_time;
                localtime_r(&time_secs, &local_time);

                var->year = local_time.tm_year + 1900;
                var->month = local_time.tm_mon + 1;
                var->day = local_time.tm_mday;
                var->hour = local_time.tm_hour;
                var->minute = local_time.tm_min;
                var->second = local_time.tm_sec;

                remainingBytes = bufferLength = sizeof(tagTIMESTAMP_STRUCT);
                break;
            }

            case SQL_C_TIME:
            case SQL_TYPE_TIME: {
                bufferPtr = (char*)bufferPtr + (sizeof(tagTIME_STRUCT) * rowIndex);
                NuoDB::Time*    time = RESULTS(getTime(column));
                time_t          time_secs = time->getSeconds();
                time->release();

                struct tm local_time;
                localtime_r(&time_secs, &local_time);

                tagTIME_STRUCT* var = (tagTIME_STRUCT*)bufferPtr;
                var->hour = local_time.tm_hour;
                var->minute = local_time.tm_min;
                var->second = local_time.tm_sec;

                remainingBytes = bufferLength = sizeof(tagTIME_STRUCT);
                break;
            }

            default:
                std::ostringstream message;
                message << "Optional feature not implemented, type " << cType << " not supported on column " << column;
                postError(new OdbcError(0, "HY000", message.str()));
                return SQL_ERROR;
        }

        if (RESULTS(wasNull())) {
            remainingBytes = SQL_NULL_DATA;
            bufferLength = SQL_NULL_DATA;
        } else if (bufferLength == 0 && binding->count > 0) { // offset will be > 0 when called multiple times
            exitCode = SQL_NO_DATA;
            binding->reset(); // at the end so reset
        } else {
            binding->count++;
        }

        if (indicatorPtr) {
            *((SQLLEN*)indicatorPtr) = indicatorIsRemaining ? remainingBytes : bufferLength;
        }
    } catch (SQLException& e) {
        if (e.getSqlcode() == TRUNCATION_ERROR) {
            postError("01004", e);

            return sqlSuccess(exitCode);
        }

        // Never Throw.
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(e.getSqlcode()), e);
        return SQL_ERROR;
    }

    return sqlSuccess(exitCode);
}

RETCODE OdbcStatement::sqlColumns(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* table, SQLSMALLINT tableLength, SQLCHAR* column, SQLSMALLINT columnLength)
{
    clearErrors();
    releaseStatement();
    char temp[1024], * p = temp;
    sqlStmt = "sqlColumns";

    const char* cat = getString(&p, catalog, catLength, NULL);
    const char* scheme = getString(&p, schema, schemaLength, NULL);
    const char* tbl = getString(&p, table, tableLength, NULL);
    const char* col = getString(&p, column, columnLength, NULL);

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        auto rSet = new ResultSetMapper(dbMetaData->getColumns(cat, scheme, tbl, col), new OdbcTypeMapper());
        setResultSet(rSet);
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlFreeStmt(SQLUSMALLINT option)
{
    clearErrors();

    switch (option) {
        case SQL_CLOSE:
            releaseResultSet();
            break;

        case SQL_UNBIND:
            fetchBindings.release();
            break;

        case SQL_RESET_PARAMS:
            parameters.release();
            break;

        default:
            std::ostringstream message;
            message << "Unexpected free statement option " << option;
            postError("HY000", message.str());
            return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlStatistics(SQLCHAR* catalog, SQLSMALLINT catLength,
                                     SQLCHAR* schema, SQLSMALLINT schemaLength,
                                     SQLCHAR* table, SQLSMALLINT tableLength,
                                     SQLUSMALLINT unique, SQLUSMALLINT reservedSic)
{
    clearErrors();
    releaseStatement();
    char temp[1024], * p = temp;

    const char* cat = getString(&p, catalog, catLength, NULL);
    const char* scheme = getString(&p, schema, schemaLength, NULL);
    const char* tbl = getString(&p, table, tableLength, NULL);

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        setResultSet(dbMetaData->getIndexInfo(cat, scheme, tbl,
                                              unique == SQL_INDEX_UNIQUE,
                                              reservedSic == SQL_QUICK));
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlPrimaryKeys(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* table, SQLSMALLINT tableLength)
{
    clearErrors();
    releaseStatement();
    char temp[1024], * p = temp;
    sqlStmt = "sqlPrimaryKeys";

    const char* cat = getString(&p, catalog, catLength, NULL);
    const char* scheme = getString(&p, schema, schemaLength, NULL);
    const char* tbl = getString(&p, table, tableLength, NULL);

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        setResultSet(dbMetaData->getPrimaryKeys(cat, scheme, tbl));
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlForeignKeys(SQLCHAR* pkCatalog, SQLSMALLINT pkCatLength,
                                      SQLCHAR* pkSchema, SQLSMALLINT pkSchemaLength,
                                      SQLCHAR* pkTable, SQLSMALLINT pkTableLength,
                                      SQLCHAR* fkCatalog, SQLSMALLINT fkCatalogLength,
                                      SQLCHAR* fkSchema, SQLSMALLINT fkSchemaLength,
                                      SQLCHAR* fkTable, SQLSMALLINT fkTableLength)
{
    clearErrors();
    releaseStatement();

    postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(FEATURE_NOT_YET_IMPLEMENTED), "getCrossReference is not implemented");
    return SQL_ERROR;
}

RETCODE OdbcStatement::sqlNumResultCols(SWORD* columns)
{
    clearErrors();

    if (!metaData) {
        // if no result sets then columns are zero.  This makes the JDBC/ODBC
        // Bridge happy
        *columns = 0;
        return sqlSuccess();
    }

    try {
        *columns = metaData->getColumnCount();
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlDescribeCol(SQLUSMALLINT col, SQLCHAR* colName, SQLSMALLINT nameSize, SQLSMALLINT* nameLength, SQLSMALLINT* sqlType, SQLULEN* precision, SQLSMALLINT* scale, SQLSMALLINT* nullable)
{
    clearErrors();

    if (!metaData) {
        postError("HY010", "MetaData is not available yet");
        return SQL_ERROR;
    }

    try {
        if (colName) {
            const char* name = metaData->getColumnLabel(col);
            if (name == nullptr || *name == 0) {
                name = metaData->getColumnName(col);
            }

            setString(name, colName, nameSize, nameLength);
        }
        if (sqlType) {
            *sqlType = (int32_t)OdbcTypeMapper::mapType(metaData->getColumnType(col));
            // some system queries return an hardcoded NULL value for columns we
            // don't have in the metadata; publish them as VARCHAR columns
            if ((SqlType)*sqlType == NUOSQL_NULL) {
                *sqlType = (int32_t)NUOSQL_VARCHAR;
            }
        }
        if (scale) {
            *scale = metaData->getScale(col);
        }
        if (precision) {
            *precision = metaData->getPrecision(col);
        }
        if (nullable) {
            *nullable = metaData->isNullable(col) ? SQL_NULLABLE : SQL_NO_NULLS;
        }
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    TRACE(formatString("SQLDescribeCol %s type %d scale %d precision " SQLULEN_FMT " nullable %d",
                               colName, *sqlType, *scale, *precision, *nullable).c_str());
    return sqlSuccess();
}

RETCODE OdbcStatement::sqlGetData(SQLUSMALLINT column, SQLSMALLINT cType, SQLPOINTER pointer, SQLLEN bufferLength, SQLLEN* indicatorPointer)
{
    clearErrors();
    getDataBindings.alloc(column);
    Binding* binding = getDataBindings.getBinding(column);
    binding->cType = cType;
    binding->pointer = pointer;
    binding->bufferLength = bufferLength;
    binding->indicatorPointer = indicatorPointer;

    try {
        return setValue(binding, column, true);
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }
}

RETCODE OdbcStatement::sqlExecute()
{
    clearErrors();

    try {
        return executeStatement();
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }
}

RETCODE OdbcStatement::sqlExecuteDirect(SQLCHAR* sql, SQLINTEGER sqlLength)
{
    int retcode = sqlPrepare(sql, sqlLength);

    if (retcode && retcode != SQL_SUCCESS_WITH_INFO) {
        return retcode;
    }

    try {
        executeStatement();
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return retcode;
}

ResultSet* OdbcStatement::getResultSet()
{
    eof = false;
    cancel = false;
    numberColumns = 0;
    rowCountPerSelect = 0;

    if (!returnedGeneratedKeys) {
        returnedGeneratedKeys = true;

        NuoDB::ResultSet* generatedKeys = statement->getGeneratedKeys();
        if (generatedKeys->getMetaData()->getColumnCount() > 0) {
            setResultSet(generatedKeys);
            return resultSet;
        }
    }

    if (!statement->getMoreResults()) {
        return NULL;
    }
    setResultSet(statement->getResultSet());

    return resultSet;
}

RETCODE OdbcStatement::sqlMoreResults()
{
    if (statement == NULL) {
        return SQL_NO_DATA;
    }
    ResultSet* resultSet = getResultSet();
    return resultSet != NULL ? sqlSuccess() : SQL_NO_DATA;
}

RETCODE OdbcStatement::sqlNumParameters(SQLSMALLINT* numParams)
{
    clearErrors();
    ParameterMetaData* pMetaData = statement->getParameterMetaData();
    if (numParams) {
        *numParams = pMetaData->getParameterCount();
    }
    return sqlSuccess();
}

RETCODE OdbcStatement::sqlDescribeParam(SQLUSMALLINT parameter, SQLSMALLINT* sqlType, SQLULEN* precision, SQLSMALLINT* scale, SQLSMALLINT* nullable)
{
    clearErrors();
    ParameterMetaData* pMetaData = statement->getParameterMetaData();

    if (sqlType) {
        *sqlType = (SQLSMALLINT)OdbcTypeMapper::mapType(pMetaData->getParameterType(parameter));
    }

    if (precision) {
        *precision = pMetaData->getPrecision(parameter);
    }

    if (scale) {
        *scale = pMetaData->getScale(parameter);
    }

    if (nullable) {
        *nullable = pMetaData->isNullable(parameter);
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlBindParameter(SQLUSMALLINT parameter,
                                        SQLSMALLINT type,
                                        SQLSMALLINT cType,
                                        SQLSMALLINT sqlType,
                                        SQLULEN columnSize,
                                        SQLSMALLINT decimalDigits,
                                        SQLPOINTER ptr,
                                        SQLLEN bufferLength,
                                        SQLLEN* length)
{
    clearErrors();

    if (parameter == 0) {
        return sqlReturn(SQL_ERROR, "S1093", "Invalid parameter number");
    }

    int parametersNeeded = parameter;

    if (parameter > parameters.getCount()) {
        if (statement) {
            ParameterMetaData*  pMetaData = statement->getParameterMetaData();
            int                 n = pMetaData->getParameterCount();
            parametersNeeded = std::max<int>(parametersNeeded, n);
        }
        parameters.alloc(parametersNeeded);
    }

    switch (cType) {
        case SQL_C_CHAR:
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
        case SQL_C_SHORT:
        case SQL_C_SLONG:
        case SQL_C_ULONG:
        case SQL_C_LONG:
        case SQL_C_FLOAT:
        case SQL_C_DOUBLE:
        case SQL_C_BIT:
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
        case SQL_C_TINYINT:
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
        case SQL_C_BINARY:
        // case SQL_C_BOOKMARK:
        // case SQL_C_VARBOOKMARK:
        case SQL_C_TYPE_DATE:
        case SQL_C_TYPE_TIME:
        case SQL_C_TYPE_TIMESTAMP:
        case SQL_C_NUMERIC:
        // case SQL_C_GUID:
        case SQL_C_DEFAULT:
            break;

        default:
            std::ostringstream msg;
            msg << "Invalid bind parameter type " << cType;
            return sqlReturn(SQL_ERROR, "HY03", msg.str().c_str());
    }

    if (bufferLength < 0) {
        std::ostringstream msg;
        msg << "Invalid buffer length " << bufferLength;
        return sqlReturn(SQL_ERROR, "HY090", msg.str().c_str());
    }

    Binding* binding = parameters.getBinding(parameter);
    binding->type = type;
    binding->cType = cType;
    binding->sqlType = sqlType;
    binding->pointer = ptr;
    binding->indicatorPointer = length;
    binding->offset = 0;

    bool hasDataAtExec = (length != 0) && (*length == SQL_DATA_AT_EXEC || *length < SQL_LEN_DATA_AT_EXEC_OFFSET);
    switch (sqlType) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
            if (type == SQL_PARAM_INPUT && !hasDataAtExec) {
                // This mimics behavior of mysql & postgres... sort of.
                // Both db's seem to only care about the strlen of the
                // input string.  mysql will put garbage in the db if
                // the string is not null terminated.
                //
                // We are doing something a little bit different.
                // We'll use the strlen() of the buffer but only up to
                // the columnSize of the buffer given.  Mysql just seems
                // to always ignore the columnSize of the buffer.
                //
                // may need to revisit with wide chars
                binding->bufferLength = ptr ? strnlen((const char*)ptr, columnSize) : 0;
            } else {
                binding->bufferLength = bufferLength;
            }
            break;

        case SQL_TIME:
        case SQL_TYPE_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
        case SQL_BINARY:
        case SQL_DATE:
        case SQL_TYPE_DATE:
            if (type == SQL_PARAM_INPUT) {
                binding->bufferLength = columnSize;
            } else {
                binding->bufferLength = bufferLength;
            }
            break;

        default:
            binding->bufferLength = bufferLength;
    }

    TRACE(formatString("bindparam %d, columnsize, %d bufsize %d, hasDataExec %s, binding buf length %d", parameter, columnSize, bufferLength, hasDataAtExec ? "true" : "false", binding->bufferLength).c_str());

    return sqlSuccess();
}

bool OdbcStatement::checkParameterSize(Binding* binding, int parameter, SQLLEN expectedSize)
{
    if (binding->bufferLength != expectedSize) {
        std::ostringstream message;
        message << "parameter " << parameter << ": expected buffer size of " << expectedSize << ", got " << binding->bufferLength;
        postError(new OdbcError(0, "HYC00", message.str()));
        return false;
    } else {
        return true;
    }
}

RETCODE OdbcStatement::setParameter(Binding* binding, int parameter)
{
    clearErrors();

    SQLLEN length = binding->indicatorPointer ? *((SQLLEN*)binding->indicatorPointer) : binding->bufferLength;

    if (length == SQL_DATA_AT_EXEC) {
        binding->dataAtExecLength = SQL_DATA_AT_EXEC;
        return SQL_NEED_DATA;
    } else if (length <= SQL_LEN_DATA_AT_EXEC_OFFSET) {
        binding->dataAtExecLength = -length + SQL_LEN_DATA_AT_EXEC_OFFSET;
        return SQL_NEED_DATA;
    } else {
        binding->dataAtExecLength = 0;
    }

    return setParameter(binding, parameter, binding->pointer, length, true);
}

RETCODE OdbcStatement::setParameter(Binding* binding, int paramId, PTR pointer, SQLLEN length, bool forceReset)
{
    int cType = binding->cType;
    if (cType == SQL_C_DEFAULT) {
        cType = convertFromSQL_C_DEFAULT((int)OdbcTypeMapper::mapType(statement->getParameterMetaData()->getParameterType(paramId)));
    }

    if (!pointer || length == SQL_NULL_DATA) {
        statement->setNull(paramId, binding->sqlType);
    } else {
        switch (cType) {
            case SQL_C_CHAR: {
                // since we append here, we need to force a reset sometimes
                std::string_view strToAppend((const char*)pointer, length == SQL_NTS ? strlen((const char*)pointer) : length);
                if (forceReset) {
                    binding->accumulator = strToAppend;
                } else {
                    binding->accumulator += strToAppend;
                }
                statement->setString(paramId, binding->accumulator.c_str(), binding->accumulator.size());
                break;
            }

            case SQL_C_BINARY: {
                std::string_view strToAppend((const char*)pointer, length);
                if (forceReset) {
                    binding->accumulator = strToAppend;
                } else {
                    binding->accumulator += strToAppend;
                }
                statement->setBytes(paramId, (int)binding->accumulator.size(), binding->accumulator.c_str());
                break;
            }

            case SQL_C_SSHORT:
            case SQL_C_USHORT:
            case SQL_C_SHORT:
                statement->setShort(paramId, *(short*)pointer);
                break;

            case SQL_C_SLONG:
            case SQL_C_ULONG:
            case SQL_C_LONG:
                statement->setInt(paramId, *(int32_t*)pointer);
                break;

            case SQL_C_FLOAT:
                statement->setFloat(paramId, *(float*)pointer);
                break;

            case SQL_C_DOUBLE:
                statement->setDouble(paramId, *(double*)pointer);
                break;

            case SQL_C_STINYINT:
            case SQL_C_UTINYINT:
            case SQL_C_TINYINT:
                statement->setByte(paramId, *(char*)pointer);
                break;

            case SQL_C_SBIGINT:
            case SQL_C_UBIGINT:
                statement->setLong(paramId, *(int64_t*)pointer);
                break;

            case SQL_C_TYPE_DATE: {
                struct tagDATE_STRUCT*  date = (tagDATE_STRUCT*)pointer;
                char dateStr[32];
                sprintf(dateStr, "%04d-%02d-%02d", date->year, date->month, date->day);
                statement->setString(paramId, dateStr);
                break;
            }

            case SQL_C_TYPE_TIME: {
                struct tagTIME_STRUCT*  time = (tagTIME_STRUCT*)pointer;
                char timeStr[32];
                sprintf(timeStr, "%02d:%02d:%02d", time->hour, time->minute, time->second);
                statement->setString(paramId, timeStr);
                break;
            }

            case SQL_C_TYPE_TIMESTAMP: {
                struct tagTIMESTAMP_STRUCT* timeStamp = (tagTIMESTAMP_STRUCT*)pointer;
                char timestampStr[64];
                sprintf(timestampStr, "%04d-%02d-%02d %02d:%02d:%02d", timeStamp->year, timeStamp->month, timeStamp->day, timeStamp->hour, timeStamp->minute, timeStamp->second);
                statement->setString(paramId, timestampStr);
                break;
            }

            case SQL_C_BIT:

            // case SQL_C_BOOKMARK:
            // case SQL_C_VARBOOKMARK:
            case SQL_C_NUMERIC:
            // case SQL_C_GUID:
            // break;

            default:
                std::ostringstream message;
                message << "set parameter type " << cType << " not implemented";
                postError(new OdbcError(0, "HYC00", message.str()));
                return SQL_ERROR;
        }
    }

    return SQL_SUCCESS;
}

RETCODE OdbcStatement::sqlCancel()
{
    clearErrors();
    cancel = true;

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlProcedures(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* proc, SQLSMALLINT procLength)
{
    clearErrors();
    releaseStatement();
    char temp[1024], * p = temp;
    sqlStmt = "sqlProcedures";

    const char* cat = getString(&p, catalog, catLength, NULL);
    const char* scheme = getString(&p, schema, schemaLength, NULL);
    const char* procedures = getString(&p, proc, procLength, NULL);

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        setResultSet(dbMetaData->getProcedures(cat, scheme, procedures));
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlProcedureColumns(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* proc, SQLSMALLINT procLength, SQLCHAR* col, SQLSMALLINT colLength)
{
    clearErrors();
    releaseStatement();
    char temp[1024], * p = temp;
    sqlStmt = "sqlProcedureColumns";

    const char* cat = getString(&p, catalog, catLength, NULL);
    const char* scheme = getString(&p, schema, schemaLength, NULL);
    const char* procedures = getString(&p, proc, procLength, NULL);
    const char* columns = getString(&p, col, colLength, NULL);

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        auto rSet = new ResultSetMapper(dbMetaData->getProcedureColumns(cat, scheme, procedures, columns), new OdbcTypeMapper());
        setResultSet(rSet);
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlCloseCursor()
{
    clearErrors();

    try {
        releaseResultSet();
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlGetStmtAttr(SQLINTEGER attribute, SQLPOINTER ptr, SQLINTEGER bufferLength, SQLINTEGER* lengthPtr)
{
    clearErrors();
    SQLULEN value;
    char*   string = NULL;

    switch (attribute) {
        case SQL_ATTR_APP_ROW_DESC:
            value = (SQLULEN)applicationRowDescriptor.get();
            break;

        case SQL_ATTR_APP_PARAM_DESC:
            value = (SQLULEN)applicationParamDescriptor.get();
            break;

        case SQL_ATTR_IMP_ROW_DESC:
            value = (SQLULEN)implementationRowDescriptor.get();
            break;

        case SQL_ATTR_IMP_PARAM_DESC:
            value = (SQLULEN)implementationParamDescriptor.get();
            break;

        case SQL_ATTR_CURSOR_TYPE:
            value = SQL_CURSOR_FORWARD_ONLY;
            break;

        case SQL_CONCURRENCY:
            value = SQL_CONCUR_LOCK;
            break;

        case SQL_ATTR_MAX_LENGTH:
            value = 0;
            break;

        case SQL_ATTR_MAX_ROWS:
            value = (SQLULEN)maxRowsPerSelect;
            break;

        case SQL_ATTR_ROW_NUMBER:
            value = (SQLULEN)rowCountPerFetch;
            break;

        case SQL_ATTR_ROW_BIND_TYPE:
            value = (SQLULEN)rowSize;
            break;

        case SQL_ATTR_ROW_ARRAY_SIZE:
            value = (SQLULEN)rowArraySize;
            break;

        case SQL_ATTR_QUERY_TIMEOUT:
            value = (SQLULEN)queryTimeoutSeconds;
            break;

        /***
            case SQL_ATTR_ASYNC_ENABLE              4
            case SQL_ATTR_CONCURRENCY               SQL_CONCURRENCY 7
            case SQL_ATTR_CURSOR_TYPE               SQL_CURSOR_TYPE
            case    SQL_ATTR_ENABLE_AUTO_IPD            15
            case SQL_ATTR_FETCH_BOOKMARK_PTR            16
            case SQL_ATTR_KEYSET_SIZE               SQL_KEYSET_SIZE
            case SQL_ATTR_NOSCAN                        SQL_NOSCAN
            case SQL_ATTR_PARAM_BIND_OFFSET_PTR     17
            case    SQL_ATTR_PARAM_BIND_TYPE            18
            case SQL_ATTR_PARAM_OPERATION_PTR       19
            case SQL_ATTR_PARAM_STATUS_PTR          20
            case    SQL_ATTR_PARAMS_PROCESSED_PTR       21
            case    SQL_ATTR_PARAMSET_SIZE              22
            case SQL_ATTR_RETRIEVE_DATA             SQL_RETRIEVE_DATA
            case SQL_ATTR_ROW_BIND_OFFSET_PTR       23

            case SQL_ATTR_ROW_NUMBER                    SQL_ROW_NUMBER
            case SQL_ATTR_ROW_OPERATION_PTR         24
            case    SQL_ATTR_ROW_STATUS_PTR             25
            case SQL_ATTR_SIMULATE_CURSOR           SQL_SIMULATE_CURSOR
            case SQL_ATTR_USE_BOOKMARKS             SQL_USE_BOOKMARKS
        ***/
        default:
            std::ostringstream msg;
            msg << "Optional feature not implemented: get statement attribute " << attribute;
            return sqlReturn(SQL_ERROR, "HYC00", msg.str().c_str());
    }

    if (string) {
        return returnStringInfo(ptr, bufferLength, lengthPtr, string);
    }

    if (ptr) {
        *(SQLULEN*)ptr = value;
    }

    if (lengthPtr) {
        *lengthPtr = sizeof(SQLULEN);
    }

    return sqlSuccess();
}

bool OdbcStatement::isStoredProcedureEscape(const char* sqlString)
{
    const char* p = sqlString;
    char        token[128];
    getToken(&p, token);

    // If we begin with "execute" assume a stored procedure

    if (strcasecmp(token, "call") == 0 || strcasecmp(token, "execute") == 0) {
        return true;
    }

    // if we're not an escape at all, bail out

    if (token[0] != '{') {
        return false;
    }

    getToken(&p, token);

    if (token[0] == '?') {
        if (*getToken(&p, token) != '=') {
            return false;
        }
        getToken(&p, token);
    }

    return strcasecmp(token, "call") == 0 || strcasecmp(token, "execute") == 0;
}

char* OdbcStatement::getToken(const char** ptr, char* token)
{
    const char* p = *ptr;
    SKIP_WHITE(p);
    char* q = token;

    if (*p) {
        char c = ODBC_STATEMENT::charTable[(int)*p];
        *q++ = *p++;
        if (c & IDENT) {
            while (ODBC_STATEMENT::charTable[(int)*p] & IDENT) {
                *q++ = *p++;
            }
        }
    }

    *q = 0;
    *ptr = p;

    return token;
}

RETCODE OdbcStatement::executeStatement()
{
    RETCODE paramCode = SQL_SUCCESS;
    currentPutDataParam = 1;

    parameters.reset();

    int paramCount = statement->getParameterMetaData()->getParameterCount();
    for (int n = 1; n <= std::min<int>(paramCount, parameters.getCount()); ++n) {
        Binding* binding = parameters.getBinding(n);

        if (binding->type != SQL_PARAM_OUTPUT) {
            switch (setParameter(binding, n)) {
                case SQL_ERROR:
                    return SQL_ERROR;

                case SQL_SUCCESS:
                    break;

                case SQL_NEED_DATA:
                    paramCode = SQL_NEED_DATA;
                    break;

                default:
                    postError("HY000", "During parameter binding unknown result return");
                    return SQL_ERROR;
            }
        }
    }

    if (paramCode == SQL_NEED_DATA) {
        return SQL_NEED_DATA;
    }

    statement->setQueryTimeout(queryTimeoutSeconds);

    return doExecuteStatement();
}

RETCODE OdbcStatement::doExecuteStatement()
{
    if (callableStatement) {
        for (int n = 1; n <= parameters.getCount(); ++n) {
            Binding* binding = parameters.getBinding(n);
            if (binding->type != SQL_PARAM_INPUT) {
                callableStatement->registerOutParameter(n, binding->sqlType);
            }
        }
    }

    bool hasRset = statement->execute();
    connection->transactionStarted();

    if (callableStatement) {
        for (int n = 1; n <= parameters.getCount(); ++n) {
            Binding* binding = parameters.getBinding(n);
            if (binding->pointer && binding->type != SQL_PARAM_INPUT) {
                setValue(binding, n, false);
            }
        }
    }

    rowCount = statement->getUpdateCount();
    if (hasRset) {
        getResultSet();
    }
    return sqlSuccess();
}

RETCODE OdbcStatement::sqlGetTypeInfo(SQLSMALLINT dataType)
{
    clearErrors();
    releaseStatement();

    try {
        DatabaseMetaData* dbMetaData = connection->getMetaData();
        ResultSetMapper* mapper;
        auto omap = new OdbcTypeMapper();
        if (dataType != SQL_ALL_TYPES) {
            auto filt = new GetDataTypeFilter(dataType);
            mapper = new ResultSetMapper(dbMetaData->getTypeInfo(), omap, filt);
        } else {
            mapper = new ResultSetMapper(dbMetaData->getTypeInfo(), omap);
        }
        setResultSet(mapper);
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlPutData(SQLPOINTER dataPtr, SQLLEN dataPtrLength)
{
    if (currentPutDataParam < 1) {
        postError("HY010", "No input parameter identified as needing data");
        return SQL_ERROR;
    }

    Binding* binding = parameters.getBinding(currentPutDataParam);

    if (binding->dataAtExecLength == 0) {
        std::ostringstream text;
        text << "Input parameter " << currentPutDataParam << " is not in need of data";
        postError("HY010", text.str());
        return SQL_ERROR;
    }

    SQLLEN  remainingLength = binding->dataAtExecLength == SQL_DATA_AT_EXEC ? dataPtrLength : binding->dataAtExecLength;
    SQLLEN  setParamLength = dataPtrLength == SQL_NTS ? SQL_NTS : std::min<SQLLEN>(remainingLength, dataPtrLength);

    RETCODE retcode = setParameter(binding, currentPutDataParam, dataPtr, setParamLength, binding->count == 0);
    binding->count++;

    binding->dataAtExecLength = dataPtrLength == SQL_NTS ? 0 : remainingLength - setParamLength;

    if (setParamLength < dataPtrLength) {
        std::ostringstream text;
        text << "Data truncated on column " << currentPutDataParam << ", need length " << dataPtrLength << " only have " << setParamLength;
        postError("01004", text.str());
        return sqlSuccess();
    } else {
        return retcode;
    }
}

RETCODE OdbcStatement::sqlParamData(SQLPOINTER* ptr)
{
    try {
        clearErrors();

        while (currentPutDataParam <= parameters.getCount()) {
            Binding* binding = parameters.getBinding(currentPutDataParam);

            if (binding->dataAtExecLength == 0) {
                currentPutDataParam++;
            } else {
                *ptr = binding->pointer;

                return SQL_NEED_DATA;
            }
        }

        return doExecuteStatement();
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }
}

RETCODE OdbcStatement::sqlSetStmtAttr(SQLINTEGER attribute, SQLPOINTER ptr, SQLINTEGER length)
{
    clearErrors();

    switch (attribute) {
        case SQL_ATTR_PARAM_BIND_TYPE:
            bindType = (SQLULEN)ptr;
            break;

        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            paramBindOffset = ptr;
            break;

        case SQL_ATTR_ROWS_FETCHED_PTR:
            rowCountPerFetchPtr = (SQLULEN*)ptr;
            break;

        case SQL_ATTR_ROW_STATUS_PTR:
            rowStatusPtr = (SQLUSMALLINT*)ptr;
            break;

        case SQL_ATTR_MAX_ROWS:
            maxRowsPerSelect = (SQLULEN)ptr;
            break;

        case SQL_ATTR_ROW_ARRAY_SIZE:
            rowArraySize = (SQLULEN)ptr;
            break;

        case SQL_ATTR_ROW_BIND_TYPE:
            rowSize = (SQLULEN)ptr;
            break;

        case SQL_ATTR_QUERY_TIMEOUT: {
            SQLULEN t = (SQLULEN)ptr;
            queryTimeoutSeconds = (int)t;
            break;
        }

        // Some statement attributes support substitution of a similar value if the data source does not support
        // the value specified in ValuePtr. In such cases, the driver returns SQL_SUCCESS_WITH_INFO and SQLSTATE
        // 01S02 (Option value changed). For example, if Attribute is SQL_ATTR_CONCURRENCY and ValuePtr is
        // SQL_CONCUR_ROWVER, and if the data source does not support this, the driver substitutes SQL_CONCUR_VALUES
        // and returns SQL_SUCCESS_WITH_INFO. To determine the substituted value, an application calls SQLGetStmtAttr.
        case SQL_ATTR_CURSOR_TYPE: {
            SQLULEN cursorType = (SQLULEN)ptr;
            if (cursorType != SQL_CURSOR_FORWARD_ONLY) {
                std::ostringstream msg;
                msg << "Optional feature not implemented: set statement attribute SQL_ATTR_CURSOR_TYPE type " << (long)cursorType << ", only SQL_CURSOR_FORWARD_ONLY is supported";
                return sqlReturn(SQL_SUCCESS_WITH_INFO, "01S02", msg.str().c_str());
            }
            break;
        }

        case SQL_ATTR_CONCURRENCY: {
            SQLULEN concurrenceType = (SQLULEN)ptr;
            if (concurrenceType != SQL_CONCUR_READ_ONLY) {
                std::ostringstream msg;
                msg << "Optional feature not implemented: set statement attribute SQL_ATTR_CONCURRENCY type " << (long)concurrenceType << ", only SQL_CONCUR_READ_ONLY is supported";
                return sqlReturn(SQL_SUCCESS_WITH_INFO, "01S02", msg.str().c_str());
            }
            break;
        }

        case SQL_ATTR_NOSCAN: {
            // Ignore.
            break;
        }
        /***
            case SQL_ATTR_ASYNC_ENABLE              4
            case SQL_ATTR_ENABLE_AUTO_IPD           15
            case SQL_ATTR_FETCH_BOOKMARK_PTR            16
            case SQL_ATTR_KEYSET_SIZE               SQL_KEYSET_SIZE
            case SQL_ATTR_MAX_LENGTH                    SQL_MAX_LENGTH
            case SQL_ATTR_PARAM_BIND_OFFSET_PTR     17
            case SQL_ATTR_PARAM_BIND_TYPE           18
            case SQL_ATTR_PARAM_OPERATION_PTR       19
            case SQL_ATTR_PARAM_STATUS_PTR          20
            case SQL_ATTR_PARAMS_PROCESSED_PTR      21
            case SQL_ATTR_PARAMSET_SIZE             22
            case SQL_ATTR_RETRIEVE_DATA             SQL_RETRIEVE_DATA
            case SQL_ATTR_ROW_BIND_OFFSET_PTR       23
            case SQL_ATTR_ROW_BIND_TYPE             SQL_BIND_TYPE
            case SQL_ATTR_ROW_NUMBER                    SQL_ROW_NUMBER
            case SQL_ATTR_ROW_OPERATION_PTR         24
            case SQL_ATTR_SIMULATE_CURSOR           SQL_SIMULATE_CURSOR
            case SQL_ATTR_USE_BOOKMARKS             SQL_USE_BOOKMARKS
        ***/

        default:
            std::ostringstream msg;
            msg << "Optional feature not implemented: set statement attribute " << attribute;
            return sqlReturn(SQL_ERROR, "HYC00", msg.str().c_str());
    }

    return sqlSuccess();
}

RETCODE OdbcStatement::sqlRowCount(SQLLEN* rowCount)
{
    clearErrors();

    try {
        if (statement) {
            *rowCount = this->rowCount;
            this->rowCount = -1;
        } else {
            TRACE("Attempt to get rowcount on null statement");
            *rowCount = 0;
        }
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    return sqlSuccess();
}

// old odbc 2.0 version of column attributes
RETCODE OdbcStatement::sqlColAttributes(SQLUSMALLINT column, SQLUSMALLINT descType, SQLPOINTER buffer, SQLSMALLINT bufferSize, SQLSMALLINT* length, SQLLEN* valuePtr)
{
    clearErrors();

    if (!metaData) {
        postError("HY010", "MetaData is not available yet");
        return SQL_ERROR;
    }

    SQLLEN      value = 0;
    const char* string = NULL;

    try {
        switch (descType) {
            case SQL_COLUMN_LABEL:
            case SQL_COLUMN_NAME:
                string = metaData->getColumnLabel(column);
                if (string == nullptr || *string == 0) {
                    string = metaData->getColumnName(column);
                }
                break;

            case SQL_COLUMN_UNSIGNED:
                value = (metaData->isSigned(column)) ? 1 : 0;
                break;

            case SQL_COLUMN_UPDATABLE:
                value = (metaData->isWritable(column)) ? 1 : 0;
                break;

            case SQL_COLUMN_COUNT:
                value = metaData->getColumnCount();
                break;

            case SQL_COLUMN_TYPE: {
                // If we have string/char/varchar types and the display size has not been set,
                // then return LONGVARCHAR
                value = (SQLLEN)OdbcTypeMapper::mapType(metaData->getColumnType(column));
                if ((value == (int32_t)NUOSQL_CHAR || value == (int32_t)NUOSQL_VARCHAR)
                    && ( metaData->getCurrentColumnMaxLength(column) == 0)) {
                    value = (int32_t)NUOSQL_LONGVARCHAR;
                }
                break;
            }

            case SQL_COLUMN_LENGTH:
            case SQL_DESC_OCTET_LENGTH:
            case SQL_COLUMN_DISPLAY_SIZE:
                value = metaData->getCurrentColumnMaxLength(column);
                break;

            case SQL_COLUMN_PRECISION:
                value = metaData->getPrecision(column);
                break;

            case SQL_COLUMN_SCALE:
                value = metaData->getScale(column);
                break;

            case SQL_COLUMN_NULLABLE:
                value = metaData->isNullable(column);
                break;

            case SQL_COLUMN_AUTO_INCREMENT:
                value = metaData->isAutoIncrement(column);
                break;

            case SQL_COLUMN_CASE_SENSITIVE:
                value = metaData->isAutoIncrement(column);
                break;

            case SQL_COLUMN_SEARCHABLE:
                value = metaData->isSearchable(column);
                break;

            case SQL_COLUMN_TYPE_NAME:
                string = metaData->getColumnTypeName(column);
                break;

            case SQL_COLUMN_TABLE_NAME:
                string = metaData->getTableName(column);
                break;

            default:
                std::ostringstream msg;
                msg << "Column attributes descriptor type out of range: " << descType;
                return sqlReturn(SQL_ERROR, "S1091", msg.str().c_str());
        }
    } catch (SQLException& exception) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(exception.getSqlcode()), exception);
        return SQL_ERROR;
    }

    if (string) {
        setString(string, (SQLCHAR*)buffer, bufferSize, length);
    } else if (valuePtr) {
        *valuePtr = value;
    }

    return sqlSuccess();
}

// odbc 3.0 version of column attributes
RETCODE OdbcStatement::sqlColAttribute(SQLUSMALLINT column,
                                       SQLUSMALLINT fieldIdentifier,
                                       SQLPOINTER buffer,
                                       SQLSMALLINT bufferSize,
                                       SQLSMALLINT* lengthPtr,
#ifdef _WIN64
                                       SQLLEN* numericAttrPtr
#else
                                       SQLPOINTER numericAttrPtr
#endif
                                       )
{
    clearErrors();

    if (!metaData) {
        postError("HY010", "MetaData is not available yet");
        return SQL_ERROR;
    }

    SQLLEN      value = 0;
    const char* string = NULL;

    try {

        switch (fieldIdentifier) {
            case SQL_DESC_LABEL:
            case SQL_COLUMN_NAME:
            case SQL_DESC_NAME:
                string = metaData->getColumnLabel(column);
                break;

            case SQL_DESC_BASE_COLUMN_NAME:
                string = metaData->getColumnName(column);
                break;

            case SQL_DESC_UNNAMED:
                value = (metaData->getColumnLabel(column)) ? SQL_NAMED : SQL_UNNAMED;
                break;

            case SQL_DESC_UNSIGNED:
                value = (metaData->isSigned(column)) ? SQL_FALSE : SQL_TRUE;
                break;

            case SQL_DESC_UPDATABLE:
                value = (metaData->isWritable(column)) ? SQL_ATTR_WRITE : SQL_ATTR_READONLY;
                break;

            case SQL_COLUMN_COUNT:
            case SQL_DESC_COUNT:
                value = metaData->getColumnCount();
                break;

            case SQL_DESC_TYPE:
            case SQL_DESC_CONCISE_TYPE:
                // If we have string/char/varchar types and the display size has not been set,
                // then return LONGVARCHAR
                value = (SQLLEN)OdbcTypeMapper::mapType(metaData->getColumnType(column));
                if ((value == (int32_t)NUOSQL_CHAR || value == (int32_t)NUOSQL_VARCHAR)
                    && ( metaData->getCurrentColumnMaxLength(column) == 0)) {
                    value = (int32_t)NUOSQL_LONGVARCHAR;
                }
                break;

            case SQL_COLUMN_LENGTH:
            case SQL_DESC_LENGTH:
            case SQL_DESC_OCTET_LENGTH:
            case SQL_DESC_DISPLAY_SIZE:
                value = metaData->getCurrentColumnMaxLength(column);
                break;

            case SQL_COLUMN_PRECISION:
            case SQL_DESC_PRECISION:
                value = metaData->getPrecision(column);
                break;

            case SQL_COLUMN_SCALE:
            case SQL_DESC_SCALE:
                value = metaData->getScale(column);
                break;

            case SQL_COLUMN_NULLABLE:
            case SQL_DESC_NULLABLE:
                value = (metaData->isNullable(column)) ? SQL_NULLABLE : SQL_NO_NULLS;
                break;

            case SQL_DESC_FIXED_PREC_SCALE:
                value = (metaData->isCurrency(column)) ? 1 : 0;
                break;

            case SQL_DESC_AUTO_UNIQUE_VALUE:
                value = (metaData->isAutoIncrement(column)) ? 1 : 0;
                break;

            case SQL_DESC_CASE_SENSITIVE:
                value = (metaData->isCaseSensitive(column)) ? SQL_TRUE : SQL_FALSE;
                break;

            case SQL_DESC_SEARCHABLE:
                value = (metaData->isSearchable(column)) ? SQL_PRED_SEARCHABLE : SQL_PRED_NONE;
                break;

            case SQL_DESC_TYPE_NAME:
                string = metaData->getColumnTypeName(column);
                break;

            case SQL_DESC_BASE_TABLE_NAME:
            case SQL_DESC_TABLE_NAME:
                string = metaData->getTableName(column);
                break;

            case SQL_DESC_SCHEMA_NAME:
                string = metaData->getSchemaName(column);
                break;

            case SQL_DESC_CATALOG_NAME:
                string = metaData->getCatalogName(column);
                break;

            default:
                std::ostringstream msg;
                msg << "Column attributes descriptor type out of range: " << fieldIdentifier;
                return sqlReturn(SQL_ERROR, "S1091", msg.str().c_str());
        }
    } catch (SQLException& ex) {
        postError(NuoDB::NuoDBSqlConstants::nuoDBCodeToSQLSTATE(ex.getSqlcode()), ex);
        return SQL_ERROR;
    }

    if (string) {
        setString(string, (SQLCHAR*)buffer, bufferSize, lengthPtr);
    } else if (numericAttrPtr) {
#ifdef _WIN64
        *numericAttrPtr = value;
#else
        *(SQLLEN*)numericAttrPtr = value;
#endif
    }

    return sqlSuccess();
}

int OdbcStatement::convertFromSQL_C_DEFAULT(int sqlType)
{
    int resultType = sqlType;
    switch (sqlType) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            resultType = SQL_C_CHAR;
            break;

        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            resultType = SQL_C_WCHAR;
            break;

        case SQL_BIT:
            resultType = SQL_C_BIT;
            break;

        case SQL_TINYINT:
            resultType = SQL_C_STINYINT;
            break;

        case SQL_SMALLINT:
            resultType = SQL_C_SSHORT;
            break;

        case SQL_INTEGER:
            resultType = SQL_C_SLONG;
            break;

        case SQL_BIGINT:
            resultType = SQL_C_CHAR;
            break;

        case SQL_REAL:
            resultType = SQL_C_FLOAT;
            break;

        case SQL_FLOAT:
        case SQL_DOUBLE:
            resultType = SQL_C_DOUBLE;
            break;

        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
            resultType = SQL_C_BINARY;
            break;

        case SQL_DATE:
            resultType = SQL_C_DATE;
            break;

        case SQL_TIME:
            resultType = SQL_C_TIME;
            break;

        case SQL_TIMESTAMP:
            resultType = SQL_C_TIMESTAMP;
            break;

        case SQL_TYPE_DATE:
            resultType = SQL_C_TYPE_DATE;
            break;

        case SQL_TYPE_TIME:
            resultType = SQL_C_TYPE_TIME;
            break;

        case SQL_TYPE_TIMESTAMP:
            resultType = SQL_C_TYPE_TIMESTAMP;
            break;

        case SQL_INTERVAL_YEAR:
            resultType = SQL_C_INTERVAL_YEAR;
            break;

        case SQL_INTERVAL_MONTH:
            resultType = SQL_C_INTERVAL_MONTH;
            break;

        case SQL_INTERVAL_DAY:
            resultType = SQL_C_INTERVAL_DAY;
            break;

        case SQL_INTERVAL_HOUR:
            resultType = SQL_C_INTERVAL_HOUR;
            break;

        case SQL_INTERVAL_MINUTE:
            resultType = SQL_C_INTERVAL_MINUTE;
            break;

        case SQL_INTERVAL_SECOND:
            resultType = SQL_C_INTERVAL_SECOND;
            break;

        case SQL_INTERVAL_YEAR_TO_MONTH:
            resultType = SQL_C_INTERVAL_YEAR_TO_MONTH;
            break;

        case SQL_INTERVAL_DAY_TO_HOUR:
            resultType = SQL_C_INTERVAL_DAY_TO_HOUR;
            break;

        case SQL_INTERVAL_DAY_TO_MINUTE:
            resultType = SQL_C_INTERVAL_DAY_TO_MINUTE;
            break;

        case SQL_INTERVAL_DAY_TO_SECOND:
            resultType = SQL_C_INTERVAL_DAY_TO_SECOND;
            break;

        case SQL_INTERVAL_HOUR_TO_MINUTE:
            resultType = SQL_C_INTERVAL_HOUR_TO_MINUTE;
            break;

        case SQL_INTERVAL_HOUR_TO_SECOND:
            resultType = SQL_C_INTERVAL_HOUR_TO_SECOND;
            break;

        case SQL_INTERVAL_MINUTE_TO_SECOND:
            resultType = SQL_C_INTERVAL_MINUTE_TO_SECOND;
            break;
    }
    return resultType;
}

#undef STATEMENT_IMPL
#undef RESULTS
#undef SKIP_WHITE
#undef PUNCT
#undef WHITE
#undef DIGIT
#undef LETTER
#undef IDENT
#undef MAX_COLUMN_NUM_SQLBIND_WITHOUT_METADATA
