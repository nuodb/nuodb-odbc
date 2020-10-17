/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "ServiceManager.h"
#include "../SetupAttributes.h"

#include "NuoRemote/Connection.h"
#include "NuoRemote/Properties.h"

ServiceManager* ServiceManager::createServices()
{
    return new ServiceManager(NuoDB_createConnection());
}

void ServiceManager::closeService()
{
    // do nothing for now
}

NuoDB::Properties* ServiceManager::allocProperties()
{
    return conn->allocProperties();
}

int ServiceManager::release()
{
    if (--useCount == 0) {
        delete this;
    }

    return useCount;
}

int ServiceManager::getDriverBuildKey()
{
    return NUOODBC_BUILD_KEY;
}
