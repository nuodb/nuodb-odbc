/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

namespace NuoDB {
class Connection;
class Properties;
}

class ServiceManager
{
public:
    int  release();
    void closeService();
    int  getDriverBuildKey();
    NuoDB::Properties* allocProperties();

    static ServiceManager* createServices();

private:
    ServiceManager(NuoDB::Connection* conn) : conn(conn) {}
    virtual ~ServiceManager() = default;

    NuoDB::Connection* conn;
    int useCount = 1;
};
