/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <string>

#include "OdbcBase.h"

class OdbcError
{
public:
    OdbcError(int code, const char* state, std::string errorMsg);
    virtual ~OdbcError() = default;

    RETCODE sqlGetDiagField(SQLSMALLINT diagId, SQLPOINTER ptr, SQLSMALLINT bufferLength, SQLSMALLINT* stringLength);
    RETCODE sqlError(SQLCHAR* stateBuffer, SQLINTEGER* nativeCode, SQLCHAR* msgBuffer, SQLSMALLINT msgBufferLength, SQLSMALLINT* msgLength);

    OdbcError*  next = nullptr;
    std::string sqlState;
    std::string msg;
    int nativeCode;
};
