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
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 1999, 2000, 2001 James A. Starkey  All Rights Reserved.
 *  (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 */

#pragma once

#include <string>

#include "OdbcBase.h"

class Setup
{
    enum
    {
        enOpenDb    = 0x00,
    };

public:
    Setup(HWND windowHandle, const char* drvr, const char* attr);
    ~Setup();

    bool configureDialog();
    void getParameters();
    bool removeDsn();
    bool addDsn();
    void configDsn();

    void readAttributes();
    void writeAttributes();
    void writeAttribute(const char* attribute, const std::string& value);

    std::string readAttribute(const char* attribute);
    std::string getAttribute(const char* attribute);

    static bool MessageBoxInstallerError(const char* stageExecuted, const char* pathOut);
    static void addKeyValue(char*& strTemp, const char* key, const char* value="");
    static void setupInstallKeys(char* buffer, char* driverPath);
    static int  installServer();

    static std::string getInstallerError();

private:
    HWND hWnd;
    std::string driver;
    std::string dsn;
    std::string description;
    std::string dbName;
    std::string user;
    std::string password;
    std::string schema;
    const char* attributes;
    ULONG serviceDb;
};
