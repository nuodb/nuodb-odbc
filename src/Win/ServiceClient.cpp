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

#include "ServiceClient.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <odbcinst.h>

#include <string>
#include <sstream>

#include "../SetupAttributes.h"
#include "OdbcSetup.h"
#include "DsnDialog.h"
#include "Setup.h"

#include "NuoRemote/Connection.h"
#include "NuoRemote/Properties.h"
#include "SQLException.h"

using namespace NuoDB;

CServiceClient::CServiceClient()
{
    services = NULL;
    properties = NULL;
    logFile = NULL;
    executedPart = 0;
#ifdef _WINDOWS
    hSemaphore = NULL;
#endif
}

CServiceClient::~CServiceClient()
{
#ifdef _WINDOWS
    if (hSemaphore) {
        CloseHandle(hSemaphore);
    }
#endif

    if (logFile) {
        fclose(logFile);
    }

    properties->release();

    if (services) {
        services->closeService();
        services->release();
    }
}

bool CServiceClient::initServices(const char* sharedLibrary)
{
    try {
        services = ServiceManager::createServices();

        if (!services) {
            return false;
        }

        properties = services->allocProperties();

        if (!properties) {
            return false;
        }
    } catch (SQLException&) {
        if (services) {
            services->release();
            services = NULL;
            return false;
        }
    }

    return true;
}

bool CServiceClient::checkVersion()
{
    if (NUOODBC_BUILD_KEY != services->getDriverBuildKey()) {
        return false;
    }

    return true;
}

void CServiceClient::putParameterValue(const char* name, const std::string& value)
{
    properties->putValue(name, value.c_str());
}

bool CServiceClient::openDatabase(std::string* errorMessage)
{
    Connection* connection;

    try {
        connection = NuoDB_createConnection();
        connection->openDatabase(properties->findValue(SETUP_DBNAME, NULL),
                                 properties);
        connection->close();
    } catch (SQLException& exception) {
        if (errorMessage) {
            *errorMessage = exception.getText();
        }

        if (connection) {
            connection->close();
        }

        return false;
    }

    return true;
}

void CServiceClient::closeService()
{
    services->closeService();
}
