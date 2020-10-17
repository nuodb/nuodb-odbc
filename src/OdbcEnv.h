/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "OdbcBase.h"
#include "OdbcObject.h"

class OdbcConnection;

class OdbcEnv : public OdbcObject
{
public:
    OdbcEnv();
    ~OdbcEnv() = default;

    RETCODE allocHandle(int handleType, SQLHANDLE* outputHandle) final;
    OdbcObjectType getType() final { return odbcTypeEnv; }

    RETCODE sqlSetEnvAttr(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER length);
    RETCODE sqlGetEnvAttr(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER length, SQLINTEGER* stringLength);
    RETCODE sqlEndTran(int operation);
    void    connectionClosed(OdbcConnection* connection);

    OdbcConnection* connections = nullptr;
    const char*     odbcIniFileName;
};
