/*
 *     The contents of this file are subject to the Initial
 *     Developer's Public License Version 1.0 (the "License");
 *     you may not use this file except in compliance with the
 *     License. You may obtain a copy of the License at
 *     http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *     Software distributed under the License is distributed on
 *     an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
 *     express or implied.  See the License for the specific
 *     language governing rights and limitations under the License.
 *
 *
 *  The Original Code was created by Vladimir Tsvigun for IBPhoenix.
 *
 *  Copyright (c) 2005 Vladimir Tsvigun  All Rights Reserved.
 *  (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 */

#pragma once

#include <cstdio>
#include <string>

#include "OdbcBase.h"
#include "ServiceManager.h"

namespace NuoDB {
class Properties;
}

class CServiceClient
{
    enum enumRestoreExecutedPart
    {
        enDomains            = 0x0001,
        enTables             = 0x0002,
        enFunctions          = 0x0004,
        enGenerators         = 0x0008,
        enStoredProcedures   = 0x0010,
        enExceptions         = 0x0020,
        enDataForTables      = 0x0040,
        enTriggers           = 0x0080,
        enPrivileges         = 0x0100,
        enSqlRoles           = 0x0200
    };

    HINSTANCE libraryHandle;

public:
    bool initServices(const char* sharedLibrary=nullptr);
    bool checkVersion(void);
    bool openDatabase(std::string* errorMessage=nullptr);
    void closeService();
    void putParameterValue(const char* name, const std::string& value);

public:
    CServiceClient(void);
    ~CServiceClient(void);

    ServiceManager*     services;
    NuoDB::Properties*  properties;
    FILE*  logFile;
    ULONG  executedPart;
    HANDLE hSemaphore;
};
