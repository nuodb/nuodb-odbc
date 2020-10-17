/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include <climits>
#include <cstring>
#include <algorithm>

#include "OdbcObject.h"

#include "OdbcBase.h"
#include "OdbcError.h"
#include "OdbcTrace.h"

#include "SQLException.h"


RETCODE OdbcObject::returnStringInfo(SQLPOINTER ptr, SQLSMALLINT maxLength, SQLSMALLINT* returnLength, const char* value)
{
    SQLLEN count = value ? (SQLLEN)strlen(value) : 0;
    *returnLength = (SQLSMALLINT)(count > SHRT_MAX ? SHRT_MAX : count);
    --maxLength;

    if (ptr) {
        if (count <= maxLength) {
            if (value) {
                strcpy((char*)ptr, value);
            }
            return sqlSuccess();
        }

        memcpy(ptr, value, maxLength);
        ((char*)ptr)[maxLength] = 0;
        *returnLength = maxLength;
    }

    return sqlReturn(SQL_SUCCESS_WITH_INFO, "01004", "String data, right truncated");
}

RETCODE OdbcObject::returnStringInfo(SQLPOINTER ptr, SQLINTEGER maxLength, SQLINTEGER* returnLength, const char* value)
{
    SQLLEN count = value ? (SQLLEN)strlen(value) : 0;
    *returnLength = (SQLINTEGER)(count > INT_MAX ? INT_MAX : count);
    --maxLength;

    if (ptr) {
        if (count <= maxLength) {
            if (value) {
                strcpy((char*)ptr, value);
            }
            return sqlSuccess();
        }

        memcpy(ptr, value, maxLength);
        ((char*)ptr)[maxLength] = 0;
        *returnLength = maxLength;
    }

    return sqlReturn(SQL_SUCCESS_WITH_INFO, "01004", "String data, right truncated");
}

int OdbcObject::sqlReturn(int code, const char* state, const char* text, int nativeCode)
{
    postError(new OdbcError(nativeCode, state, text));

    return code;
}

int OdbcObject::sqlSuccess(int exitCode)
{
    if (infoPosted) {
        return SQL_SUCCESS_WITH_INFO;
    }

    return exitCode;
}

RETCODE OdbcObject::allocHandle(int handleType, SQLHANDLE* outputHandle)
{
    *outputHandle = NULL;

    return sqlReturn(SQL_ERROR, "HY092", "Invalid attribute/option identifier");
}

void OdbcObject::notYetImplemented(const char* msg)
{}

SQLLEN OdbcObject::stringLength(const SQLCHAR* string, SQLSMALLINT givenLength)
{
    if (!string) {
        return 0;
    }

    if (givenLength == SQL_NTS) {
        return (SQLLEN)strlen((const char*)string);
    }

    return givenLength;
}

// Return "true" on overflow

bool OdbcObject::setString(const SQLCHAR* string, SQLLEN stringLength, SQLCHAR* target, SQLSMALLINT targetSize, SQLSMALLINT* targetLength)
{
    --targetSize;

    if (targetLength) {
        *targetLength = (SQLSMALLINT)(stringLength > SHRT_MAX ? SHRT_MAX : stringLength);
    }

    if (target) {
        if (stringLength <= targetSize) {
            memcpy(target, string, stringLength);
            target[stringLength] = 0;
            return false;
        }

        memcpy(target, string, targetSize);
        target[targetSize] = 0;
        postError(new OdbcError(0, "01004", "String data, right truncated"));
    }

    return true;
}

bool OdbcObject::setString(const char* string, SQLCHAR* target, SQLSMALLINT targetSize, SQLSMALLINT* targetLength)
{
    return setString((SQLCHAR*)string, string ? (SQLLEN)strlen(string) : 0, target, targetSize, targetLength);
}

bool OdbcObject::appendString(const char* string, SQLSMALLINT stringLength, SQLCHAR* target, SQLSMALLINT targetSize, SQLSMALLINT* targetLength)
{
    --targetSize;
    SQLSMALLINT length = *targetLength;
    *targetLength += stringLength;
    SQLSMALLINT l = targetSize - length;

    if (stringLength <= l) {
        memcpy(target + length, string, stringLength);
        target[length + stringLength] = 0;
        return false;
    }

    if (l > 0) {
        memcpy(target, string, targetSize - length);
    }

    target[targetSize] = 0;

    return true;
}

RETCODE OdbcObject::sqlError(SQLCHAR* stateBuffer, SQLINTEGER* nativeCode, SQLCHAR* msgBuffer, SQLSMALLINT msgBufferLength, SQLSMALLINT* msgLength)
{
    OdbcError* error = errors;

    if (error) {
        errors = error->next;
        RETCODE ret = error->sqlError(stateBuffer, nativeCode, msgBuffer, msgBufferLength, msgLength);
        delete error;
        return ret;
    }

    strcpy((char*)stateBuffer, "00000");
    msgBuffer[0] = 0;
    *msgLength = 0;

    return SQL_NO_DATA_FOUND;
}

void OdbcObject::postError(OdbcError* error)
{
    infoPosted = true;
    OdbcError** ptr;

    for (ptr = &errors; *ptr; ptr = &(*ptr)->next) {}

    error->next = NULL;
    *ptr = error;
}

void OdbcObject::clearErrors()
{
    TRACE("clearErrors");
    while (errors) {
        OdbcError* error = errors;
        errors = error->next;
        delete error;
    }

    infoPosted = false;
}

void OdbcObject::postError(const char* sqlState, NuoDB::SQLException& exception)
{
    postError(new OdbcError(exception.getSqlcode(), sqlState, exception.getText()));
}

const char* OdbcObject::getString(char** temp, const UCHAR* string, int length, const char* defaultValue)
{
    if (!string) {
        return defaultValue;
    }

    if (length == SQL_NTS) {
        return (char*)string;
    }

    char* ret = *temp;
    memcpy(ret, string, length);
    ret[length] = 0;
    *temp += length + 1;

    return ret;
}

void OdbcObject::postError(const char* state, std::string msg)
{
    postError(new OdbcError(0, state, msg));
}

RETCODE OdbcObject::sqlGetDiagRec(int handleType, int recNumber, SQLCHAR* stateBuffer, SQLINTEGER* nativeCode, SQLCHAR* msgBuffer, int msgBufferLength, SQLSMALLINT* msgLength)
{
    int n = 1;

    for (OdbcError* error = errors; error; error = error->next, ++n) {
        if (n == recNumber) {
            return error->sqlError(stateBuffer, nativeCode, msgBuffer, msgBufferLength, msgLength);
        }
    }

    strcpy((char*)stateBuffer, "00000");

    if (msgBuffer) {
        msgBuffer[0] = 0;
    }

    *msgLength = 0;

    return SQL_NO_DATA_FOUND;
}

RETCODE OdbcObject::sqlGetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagId, SQLPOINTER ptr, SQLSMALLINT bufferLength, SQLSMALLINT* stringLength)
{
    int n = 1;
    for (OdbcError* error = errors; error; error = error->next, ++n) {
        if (n == recNumber) {
            return error->sqlGetDiagField(diagId, ptr, bufferLength, stringLength);
        }
    }

    return SQL_NO_DATA_FOUND;
}
