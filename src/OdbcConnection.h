/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "OdbcBase.h"

#include <string>

#include "OdbcDesc.h"

namespace NuoDB {
class CallableStatement;
class Connection;
class DatabaseMetaData;
class PreparedStatement;
}

class OdbcEnv;
class OdbcStatement;

class OdbcConnection : public OdbcObject
{
public:
    OdbcConnection(OdbcEnv* parent);
    virtual ~OdbcConnection();
    void transactionStarted();

    RETCODE sqlBrowseConnect(SQLCHAR* inConnectionString,
                             SQLSMALLINT stringLength1,
                             SQLCHAR* outConnectionString,
                             SQLSMALLINT bufferLength,
                             SQLSMALLINT* stringLength2Ptr);

    RETCODE                     sqlGetConnectAttr(SQLINTEGER attribute, SQLPOINTER ptr, SQLINTEGER bufferLength, SQLINTEGER* lengthPtr);
    void                        descriptorDeleted(OdbcDesc* descriptor);
    OdbcDesc*                   allocDescriptor(OdbcDescType type);
    void                        expandConnectParameters();
    void                        statementDeleted(OdbcStatement* statement);
    RETCODE                     sqlEndTran(SQLUSMALLINT operation);
    RETCODE                     connect();
    RETCODE                     sqlConnect(const SQLCHAR* dsn, SQLSMALLINT dsnLength, SQLCHAR* UID, SQLSMALLINT uidLength, SQLCHAR* password, SQLSMALLINT passwordLength);
    NuoDB::DatabaseMetaData*    getMetaData();
    virtual RETCODE             allocHandle(int handleType, SQLHANDLE* outputHandle);
    char*                       appendAttribute(const char* name, const char* value, char* ptr, bool empty);
    char*                       appendString(char* ptr, const char* string);
    RETCODE                     sqlGetInfo(SQLUSMALLINT type, SQLPOINTER ptr, SQLSMALLINT maxLength, SQLSMALLINT* actualLength);
    RETCODE                     sqlDisconnect();
    RETCODE                     sqlGetFunctions(SQLUSMALLINT functionId, SQLUSMALLINT* supportedPtr);
    std::string                 readAttribute(const char* attribute);
    RETCODE                     sqlDriverConnect(SQLHWND hWnd,
                                                 const SQLCHAR* connectString, SQLSMALLINT connectStringLength,
                                                 SQLCHAR* outConnectBuffer, SQLSMALLINT connectBufferLength, SQLSMALLINT* outStringLength,
                                                 SQLUSMALLINT driverCompletion);
    RETCODE                     sqlSetConnectAttr(SQLINTEGER arg1, SQLPOINTER arg2, SQLINTEGER stringLength);
    virtual OdbcObjectType      getType();
    NuoDB::CallableStatement*   prepareCall(const char* sql);
    NuoDB::PreparedStatement*   prepareStatement(const char* sql);

private:
    int32_t getSupportedTransactionIsolationBitmask();
    void close();

    OdbcEnv*            env;
    NuoDB::Connection*  connection;
    OdbcStatement*      statements;
    OdbcDesc*           descriptors;
    bool                connected;
    /**
     * transactionPending is use to catch the sqlDisconnect error:
     * 25000: Invalid transaction state (see sqlDisconnect).
     * But transactionPending is lame.  It only captures if the client thinks that there
     * is a transaction open.  But a client could issue a sql: "commit" or "rollback"
     * and the client wouldn't know.  This really needs to be tracked on the server.
     * Probably a connection property needs to be set to switch the server connection into
     * a mode where it will throw if an attempt is made to close a connection with a
     * transaction open.
     */
    bool                transactionPending;
    SQLLEN              connectionTimeout;
    std::string         dsn;
    std::string         databaseName;
    std::string         account;
    std::string         password;
    std::string         schema;
    std::string         driver;
    bool                asyncEnabled;
    bool                autoCommit;
    int                 transactionIsolation;
};
