/**
 * (C) Copyright NuoDB, Inc. 2011-2014  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <string>

#include "OdbcBase.h"
#include "SQLException.h"

enum OdbcObjectType
{
    odbcTypeEnv,
    odbcTypeConnection,
    odbcTypeStatement,
    odbcTypeDescriptor,
};

class OdbcError;

class OdbcObject
{
public:
    OdbcObject() = default;
    virtual ~OdbcObject() { clearErrors(); }

    virtual OdbcObjectType getType() = 0;
    virtual RETCODE allocHandle(int handleType, SQLHANDLE* outputHandle);
    virtual RETCODE sqlGetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagId, SQLPOINTER ptr, SQLSMALLINT bufferLength, SQLSMALLINT* stringLength);
    virtual RETCODE sqlError(SQLCHAR* stateBuffer, SQLINTEGER* nativeCode, SQLCHAR* msgBuffer, SQLSMALLINT msgBufferLength, SQLSMALLINT* msgLength);

    RETCODE returnStringInfo(SQLPOINTER ptr, SQLINTEGER maxLength, SQLINTEGER* returnLength, const char* value);
    RETCODE returnStringInfo(SQLPOINTER ptr, SQLSMALLINT maxLength, SQLSMALLINT* returnLength, const char* value);
    RETCODE sqlGetDiagRec(int handleType, int recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeErrorPtr, SQLCHAR* messageText, int bufferLength, SQLSMALLINT* textLengthPtr);
    const char* getString(char** temp, const UCHAR* string, int length, const char* defaultValue);
    void postError(OdbcError* error);
    void postError(const char* state, std::string msg);
    void postError(const char* sqlState, NuoDB::SQLException& exception);
    void clearErrors();
    bool appendString(const char* string, SQLSMALLINT stringLength, SQLCHAR* target, SQLSMALLINT targetSize, SQLSMALLINT* targetLength);
    bool setString(const char* string, SQLCHAR* target, SQLSMALLINT targetSize, SQLSMALLINT* targetLength);
    bool setString(const SQLCHAR* string, SQLLEN stringLength, SQLCHAR* target, SQLSMALLINT targetSize, SQLSMALLINT* targetLength);
    SQLLEN stringLength(const SQLCHAR* string, SQLSMALLINT givenLength);

    void notYetImplemented(const char* msg);
    int sqlReturn(int code, const char* state, const char* text, int nativeCode=0);

    // return the exitCode or SQL_SUCCESS_WITH_INFO if there is info posted
    int sqlSuccess(int exitCode);
    int sqlSuccess() { return sqlSuccess(SQL_SUCCESS); }

    OdbcError*  errors = nullptr;
    OdbcObject* next = nullptr;
    bool infoPosted = false;
};
