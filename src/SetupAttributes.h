/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "ProductVersion.h"

#define DRIVER_NAME         "NuoODBC"
#define DRIVER_FULL_NAME    NUOODBC_PRODUCT
#define DRIVER_VERSION      NUOODBC_VERSION_STR
#define DRIVER_BUILD_KEY    NUOODBC_BUILD_KEY

#define SETUP_DSN           "DSN"
#define SETUP_DESCRIPTION   "Description"
#define SETUP_DBNAME        "Dbname"
#define SETUP_DATABASE      "Database"
#define SETUP_SERVERNAME    "ServerName"
#define SETUP_PORT          "Port"
#define SETUP_DRIVER        "Driver"
#define SETUP_USER          "User"
#define SETUP_PASSWORD      "Password"
#define SETUP_SCHEMA        "Schema"

#define INSTALL_DRIVER      "Driver"
#define INSTALL_SETUP       "Setup"
#define INSTALL_FILE_EXT    "FileExtns"
#define INSTALL_API_LEVEL   "APILevel"
#define INSTALL_CONNECT_FUN "ConnectFunctions"
#define INSTALL_FILE_USAGE  "FileUsage"
#define INSTALL_SQL_LEVEL   "SQLLevel"
#define INSTALL_DRIVER_VER  "DriverODBCVer"

#define VALUE_FILE_EXT      "*.fdb,*.gdb"
#define VALUE_API_LEVEL     "1"
#define VALUE_CONNECT_FUN   "YYY"
#define VALUE_FILE_USAGE    "0"
#define VALUE_DRIVER_VER    "03.51"
#define VALUE_SQL_LEVEL     "1"

#define KEY_DSN_UID         "UID"
#define KEY_DSN_PWD         "PWD"
#define KEY_DSN_SCHEMA      "SCHEMA"
#define KEY_DSN_QUOTED      "QUOTED"
#define KEY_DSN_CHARSET     "CHARSET"
#define KEY_DSN_LOGFILE     "LOGFILE"
#define KEY_DSN_SENSITIVE   "SENSITIVE"
#define KEY_DSN_AUTOQUOTED  "AUTOQUOTED"
#define KEY_DSN_DATABASE    "DATABASE"
