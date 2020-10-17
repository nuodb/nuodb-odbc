/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include <cstring>
#include <string>

#include "OdbcError.h"

#include "OdbcBase.h"
#include "OdbcTrace.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OdbcError::OdbcError(int code, const char* state, std::string errorMsg)
    : sqlState(state),
      msg(errorMsg),
      nativeCode(code)
{
    TRACE(errorMsg.c_str());
}

RETCODE OdbcError::sqlError(SQLCHAR* stateBuffer, SQLINTEGER* nativeCodePtr, SQLCHAR* msgBuffer, SQLSMALLINT msgBufferLength, SQLSMALLINT* msgLength)
{
    TRACE(formatString("SQLError state (%s) message (%s)", sqlState.c_str(), msg.c_str()).c_str());
    strcpy((char*)stateBuffer, sqlState.c_str());

    if (nativeCodePtr) {
        *nativeCodePtr = nativeCode;
    }

    SQLLEN length = msg.size() - 1;

    if (length <= msgBufferLength && msgBuffer) {
        strcpy((char*)msgBuffer, msg.c_str());
        *msgLength = (SQLSMALLINT)(length + 1);
        return SQL_SUCCESS;
    }

    if (msgBuffer) {
        memcpy(msgBuffer, msg.c_str(), msgBufferLength);
        msgBuffer[msgBufferLength] = 0;
    }
    *msgLength = (SQLSMALLINT)(length + 1);
    return SQL_SUCCESS_WITH_INFO;
}

RETCODE OdbcError::sqlGetDiagField(SQLSMALLINT diagId, SQLPOINTER ptr, SQLSMALLINT msgBufferLength, SQLSMALLINT* msgLength)
{
    const char* string = NULL;
    int         value = 0;

    switch (diagId) {
        case SQL_DIAG_CLASS_ORIGIN:
        case SQL_DIAG_SUBCLASS_ORIGIN:
            if (sqlState[0] == 'I' && sqlState[1] == 'M') {
                string = "ODBC 3.0";
            } else {
                string = "ISO 9075";
            }
            break;

        case SQL_DIAG_CONNECTION_NAME:
        case SQL_DIAG_SERVER_NAME:
            string = "";
            break;

        case SQL_DIAG_MESSAGE_TEXT:
            string = msg.c_str();
            break;

        case SQL_DIAG_NATIVE:
            value = nativeCode;
            break;

        case SQL_DIAG_SQLSTATE:
            string = sqlState.c_str();
            break;

        default:
            return SQL_ERROR;
    }

    if (!string) {
        *(SQLINTEGER*)ptr = value;
        return SQL_SUCCESS;
    }

    char*   msgBuffer = (char*)ptr;
    SQLLEN  length = strlen(string) - 1;

    if (length <= msgBufferLength) {
        strcpy((char*)msgBuffer, string);
        *msgLength = (SQLSMALLINT)(length + 1);
        return SQL_SUCCESS;
    }

    memcpy(msgBuffer, msg.c_str(), length);
    msgBuffer[length] = 0;

    return SQL_SUCCESS_WITH_INFO;
}
