/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <memory>

#include "OdbcBase.h"
#include "OdbcObject.h"
#include "Bindings.h"

namespace NuoDB {
class CallableStatement;
class PreparedStatement;
class ResultSet;
class ResultSetMetaData;
}

class OdbcConnection;
class OdbcDesc;
class RemPreparedStatement;

class OdbcStatement : public OdbcObject
{
public:
    OdbcStatement(OdbcConnection* connect);
    virtual ~OdbcStatement();

    // odbc 2.0
    RETCODE sqlColAttributes(SQLUSMALLINT column,
                             SQLUSMALLINT descType,
                             SQLPOINTER buffer,
                             SQLSMALLINT bufferSize,
                             SQLSMALLINT* length,
                             SQLLEN* value);
    // odbc 3.0
    RETCODE sqlColAttribute(SQLUSMALLINT columnNumber,
                            SQLUSMALLINT fieldIdentifier,
                            SQLPOINTER characterAttrPtr,
                            SQLSMALLINT bufferLen,
                            SQLSMALLINT* stringLengthPtr,
#ifdef _WIN64
                            SQLLEN* numericAttrPtr
#else
                            SQLPOINTER numericAttrPtr
#endif
                            );
    RETCODE                 sqlRowCount(SQLLEN* rowCount);
    RETCODE                 sqlSetStmtAttr(SQLINTEGER attribute, SQLPOINTER ptr, SQLINTEGER length);
    RETCODE                 sqlParamData(SQLPOINTER* ptr);
    RETCODE                 sqlPutData(SQLPOINTER dataPointer, SQLLEN length);
    RETCODE                 sqlGetTypeInfo(SQLSMALLINT dataType);
    RETCODE                 executeStatement();
    RETCODE                 doExecuteStatement();
    char*                   getToken(const char** ptr, char* token);
    bool                    isStoredProcedureEscape(const char* sqlString);
    RETCODE                 sqlGetStmtAttr(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* lengthPtr);
    RETCODE                 sqlCloseCursor();
    RETCODE                 sqlProcedureColumns(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* proc, SQLSMALLINT procLength, SQLCHAR* col, SQLSMALLINT colLength);
    RETCODE                 sqlProcedures(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* proc, SQLSMALLINT procLength);
    RETCODE                 sqlCancel();
    RETCODE                 setParameter(Binding* binding, int parameter);
    RETCODE                 setParameter(Binding* binding, int parameter, PTR pointer, SQLLEN length, bool forceReset);
    RETCODE                 sqlNumParameters(SQLSMALLINT* numParams);
    RETCODE                 sqlBindParameter(SQLUSMALLINT parameter, SQLSMALLINT type, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN columnSize, SQLSMALLINT decimalDigits, SQLPOINTER ptr, SQLLEN bufferLength, SQLLEN* length);
    RETCODE                 sqlDescribeParam(SQLUSMALLINT parameter, SQLSMALLINT* sqlType, SQLULEN* precision, SQLSMALLINT* scale, SQLSMALLINT* nullable);
    NuoDB::ResultSet*       getResultSet();
    RETCODE                 sqlExecuteDirect(SQLCHAR* sql, SQLINTEGER sqlLength);
    RETCODE                 sqlExecute();
    RETCODE                 sqlGetData(SQLUSMALLINT column, SQLSMALLINT cType, SQLPOINTER value, SQLLEN bufferLength, SQLLEN* length);
    RETCODE                 sqlDescribeCol(SQLUSMALLINT col, SQLCHAR* colName, SQLSMALLINT nameSize, SQLSMALLINT* nameLength, SQLSMALLINT* sqlType, SQLULEN* precision, SQLSMALLINT* scale, SQLSMALLINT* nullable);
    RETCODE                 sqlNumResultCols(SWORD* columns);
    RETCODE                 sqlForeignKeys(SQLCHAR* pkCatalog, SQLSMALLINT pkCatLength, SQLCHAR* pkSchema, SQLSMALLINT pkSchemaLength, SQLCHAR* pkTable, SQLSMALLINT pkTableLength, SQLCHAR* fkCatalog, SQLSMALLINT fkCatalogLength, SQLCHAR* fkSchema, SQLSMALLINT fkSchemaLength, SQLCHAR* fkTable, SQLSMALLINT fkTableLength);
    RETCODE                 sqlPrimaryKeys(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* table, SQLSMALLINT tableLength);
    RETCODE                 sqlStatistics(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* table, SQLSMALLINT tableLength, SQLUSMALLINT unique, SQLUSMALLINT reservedSic);
    RETCODE                 sqlFreeStmt(SQLUSMALLINT option);
    int                     setValue(Binding* binding, int column, bool indicatorIsRemaining, SQLULEN rowIndex=0, SQLULEN rowSize=SQL_BIND_BY_COLUMN);
    RETCODE                 sqlFetch();
    RETCODE                 sqlBindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, SQLPOINTER targetValuePtr, SQLLEN bufferLength, SQLLEN* indPtr);
    void                    setResultSet(NuoDB::ResultSet* results);
    void                    releaseResultSet();
    void                    releaseStatement();
    RETCODE                 sqlPrepare(SQLCHAR* sql, SQLINTEGER sqlLength);
    RETCODE                 sqlColumns(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* table, SQLSMALLINT tableLength, SQLCHAR* column, SQLSMALLINT columnLength);
    RETCODE                 sqlTables(SQLCHAR* catalog, SQLSMALLINT catLength, SQLCHAR* schema, SQLSMALLINT schemaLength, SQLCHAR* table, SQLSMALLINT tableLength, SQLCHAR* type, SQLSMALLINT typeLength);
    RETCODE                 sqlMoreResults();
    virtual OdbcObjectType  getType();
    static int              convertFromSQL_C_DEFAULT(int sqlType);

private:
    bool checkParameterSize(Binding* binding, int parameter, SQLLEN expectedSize);

    std::string     sqlStmt;

    OdbcConnection* connection;
    std::unique_ptr<OdbcDesc> applicationRowDescriptor;
    std::unique_ptr<OdbcDesc> applicationParamDescriptor;
    std::unique_ptr<OdbcDesc> implementationRowDescriptor;
    std::unique_ptr<OdbcDesc> implementationParamDescriptor;

    NuoDB::ResultSet*         resultSet = nullptr;
    NuoDB::PreparedStatement* statement = nullptr;
    NuoDB::CallableStatement* callableStatement = nullptr;
    NuoDB::ResultSetMetaData* metaData = nullptr;

    void*         paramBindOffset = nullptr;
    Bindings      fetchBindings;
    Bindings      parameters;
    Bindings      getDataBindings;
    SQLLEN        rowCount = -1;
    SQLULEN       bindType = 0;
    SQLULEN       rowCountPerFetch = 0;   // number of rows that we have fetched in this SQLFetch call
    SQLULEN*      rowCountPerFetchPtr = nullptr; // optional pointer that user uses to keep track of number of rows fetched per SQLFetch
    SQLULEN       rowCountPerSelect = 0;  // number of rows that we have fetched in this select statement
    SQLULEN       maxRowsPerSelect = 0;   // max # of rows to fetch per select like an sql limit
    SQLUSMALLINT* rowStatusPtr = nullptr; // an array used to maintain a status of a fetched row when fetching rows in groups
    SQLULEN       rowArraySize = 1;
    SQLULEN       rowSize = SQL_BIND_BY_COLUMN;
    int           numberColumns = 0;
    int           currentPutDataParam = 0;
    int           queryTimeoutSeconds = 0;
    bool          eof = false;
    bool          cancel = false;
    bool          returnedGeneratedKeys = false;
};
