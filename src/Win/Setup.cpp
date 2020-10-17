/*
 *     The contents of this file are subject to the Initial
 *     Developer's Public License Version 1.0 (the "License");
 *     you may not use this file except in compliance with the
 *     License. You may obtain a copy of the License at
 *     http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl
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
 *  (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 *  2002-06-08  Setup.cpp
 *              Added changes suggested by C. G. Alvarez to
 *              correctly locate the driver if already
 *              installed and to correctly report any errors.
 *
 *  2002-04-30  Added 'role' fix from Paul Schmidt      (PCR)
 *
 */

#include "Setup.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <sstream>

#include <odbcinst.h>

#include "../SetupAttributes.h"

#include "OdbcSetup.h"
#include "DsnDialog.h"
#include "ServiceClient.h"

extern HINSTANCE        m_hInstance;
extern int              currentCP;
extern TranslateString  translate[];

#ifdef _WINDOWS
# ifndef strncasecmp

#  if _MSC_VER >= 1400 // VC80 and later
#   define strncasecmp _strnicmp
#  else
#   define strncasecmp strnicmp
#  endif // _MSC_VER >= 1400

# endif // strncasecmp
#endif

BOOL INSTAPI ConfigDSN(HWND hWnd,
                       WORD fRequest,
                       LPCSTR lpszDriver,
                       LPCSTR lpszAttributes)
{

    if (!lpszDriver || strncmp(lpszDriver, DRIVER_FULL_NAME, strlen(DRIVER_FULL_NAME))) {
        std::ostringstream message;
        message << "Invalid driver name: " << lpszDriver;
        printf("%s\n", message.str().c_str());
        SQLPostInstallerError(ODBC_ERROR_INVALID_NAME, message.str().c_str());
        return false;
    }

    Setup setup(hWnd, lpszDriver, lpszAttributes);
    switch (fRequest) {
        case ODBC_CONFIG_DSN:
            setup.configDsn();
            break;

        case ODBC_ADD_DSN:
            if (!setup.addDsn()) {
                return false;
            }
            break;

        case ODBC_REMOVE_DSN:
            setup.removeDsn();
            break;
    }

    return TRUE;
}

#if defined __BORLANDC__ || defined __MINGW32__
extern "C" __declspec(dllexport) BOOL INSTAPI ConfigTranslator(HWND hWnd, DWORD* pvOptionWORD)
#else
BOOL INSTAPI ConfigTranslator(HWND hWnd, DWORD* pvOptionWORD)
#endif
{
    return TRUE;
}

/*
 *  Registration can be performed with the following command:
 *
 *      regsvr32 .\NuoOdbc.dll
 *
 *  To debug registration the project settings to call regsvr32.exe
 *  with the full path.
 *
 *  Use
 *      ..\debug\NuoOdbc.dll
 *
 *  as the program argument
 *
 */
#include "../OdbcTrace.h"

extern "C" STDAPI DllRegisterServer(void)
{
    return Setup::installServer();
}

static std::string getErrorText()
{
    DWORD errorId = GetLastError();

    // Avoid double-allocation and allow for 100 chars of error message
# define WIN_MESSAGE_BUFLEN 100

    char buf[WIN_MESSAGE_BUFLEN+1];
    buf[WIN_MESSAGE_BUFLEN] = '\0';

    DWORD length = FormatMessage(
        // use system message tables to retrieve error text
        FORMAT_MESSAGE_FROM_SYSTEM
        // we cannot pass insertion parameters
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        // unused with FORMAT_MESSAGE_FROM_SYSTEM
        NULL,
        errorId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        WIN_MESSAGE_BUFLEN,
        NULL);

    if (length == 0) {
        std::ostringstream message;
        message << "Failed to lookup system error message for " << errorId << ": " << GetLastError();
        return message.str();
    }

    // Some (all?) Windows system messages have a \r\n added to the end.
    if (length >= 2 && buf[length-2] == '\r' && buf[length-1] == '\n') {
        buf[length-2] = '\0';
    }

    return buf;
}

// static
int Setup::installServer()
{
    char    pathOut[MAX_PATH];
    char    pathIn[MAX_PATH];
    WORD    length = sizeof(pathOut) - 1;
    DWORD   useCount;

    std::ostringstream dllName;
    dllName << DRIVER_NAME << ".dll";
    const char* odbcDllName = dllName.str().c_str();
    HMODULE odbcDLL = GetModuleHandle(odbcDllName);
    if (odbcDLL) {
        if (!GetModuleFileName(odbcDLL, pathIn, sizeof(pathIn))) {
            std::ostringstream message;
            message << "GetModuleFileName failed: Unable to determine pathname of ODBC dll " << odbcDllName << ": " << getErrorText();
            TRACE(message.str().c_str());
            MessageBoxInstallerError(message.str().c_str(), NULL);
            return E_FAIL;
        }
    } else {
        std::ostringstream message;
        message << "GetModuleHandle failed: Unable to find handle for ODBC dll " << odbcDllName << ": " << getErrorText();
        TRACE(message.str().c_str());
        MessageBoxInstallerError(message.str().c_str(), NULL);
        return E_FAIL;
    }

    char driverInfo[1024];
    setupInstallKeys(driverInfo, pathIn);

    if (!SQLInstallDriverEx(
            driverInfo,
            NULL,
            pathOut,
            sizeof(pathOut),
            &length,
            ODBC_INSTALL_INQUIRY,
            &useCount)) {
        std::ostringstream message;
        message << "Install Driver Failed: " << getInstallerError();
        TRACE(message.str().c_str());
        MessageBoxInstallerError(message.str().c_str(), NULL);
        return E_FAIL;
    }

    std::ostringstream fullPathOut;
    fullPathOut << pathOut << '\\' << odbcDllName;

    if (!SQLInstallDriverEx(
            driverInfo,
            NULL,
            pathOut,
            sizeof(pathOut),
            &length,
            ODBC_INSTALL_COMPLETE,
            &useCount)) {
        MessageBoxInstallerError("Install Driver Failed", pathOut);
        return E_FAIL;
    }

    char message[1024];
    WORD msgLen = 0;
    if (!SQLConfigDriver(NULL, ODBC_INSTALL_DRIVER, DRIVER_FULL_NAME, 0, message, sizeof(message), &msgLen))
    {
        MessageBoxInstallerError("SQLConfigDriver failed", message);
    }
    return S_OK;
}

extern "C" HRESULT INSTAPI DllInstall(BOOL install, LPCWSTR commandLine)
{
    return DllRegisterServer();
}

extern "C" STDAPI DllUnregisterServer(void)
{
    DWORD useCount = 0;

    if (!SQLRemoveDriver(DRIVER_FULL_NAME, FALSE, &useCount)) {
        std::ostringstream msg;
        msg << "Uninstall Driver failed: " << getErrorText();
        TRACE(msg.str().c_str());
        Setup::MessageBoxInstallerError("Uninstall Driver", NULL);
        return E_FAIL;
    }

    std::ostringstream msg;
    msg << "RemoveDriver usecount " << useCount;
    TRACE(msg.str().c_str());

    return S_OK;
}

BOOL INSTAPI ConfigDriver(HWND hwndParent,
                          WORD fRequest,
                          LPCSTR lpszDriver,
                          LPCSTR lpszArgs,
                          LPSTR lpszMsg,
                          WORD cbMsgMax,
                          WORD* pcbMsgOut)
{
    TRACE("ConfigDriver");
    return TRUE;
}

bool Setup::MessageBoxInstallerError(const char* stageExecuted, const char* pathOut)
{
    RETCODE rc = SQL_SUCCESS_WITH_INFO;
    char    message[SQL_MAX_MESSAGE_LENGTH];
    WORD    errCodeIn = 1;
    DWORD   errCodeOut = 0L;
    WORD    cbErrorMsg = 0;

    std::ostringstream msg, msgTemp;
    bool hasMsg = false;

    while (rc == SQL_SUCCESS_WITH_INFO) {
        rc = SQLInstallerError(errCodeIn, &errCodeOut, message, sizeof(message) - 1, &cbErrorMsg);
        if (cbErrorMsg) {
            msgTemp << message;
            hasMsg = true;
        }
    }

    if (hasMsg) {
        if (pathOut && *pathOut) {
            msg << stageExecuted << " (" << DRIVER_FULL_NAME << ", " << pathOut << " failed with " << errCodeOut << ")\n" << msgTemp.str() << "\n";
        } else {
            msg << stageExecuted << " (" << DRIVER_FULL_NAME << " failed with " << errCodeOut << ")\n" << msgTemp.str() << "\n";
        }
        MessageBox(NULL, msg.str().c_str(), DRIVER_NAME, MB_ICONSTOP|MB_OK);
        return true;
    }

    return false;
}

void Setup::addKeyValue(char*& strTemp, const char* key, const char* value)
{
    while ((*strTemp++ = *key++)) {}

    if (*value) {
        --strTemp;
        *strTemp++ = '=';
        while ((*strTemp++ = *value++)) {}
    }
}

void Setup::setupInstallKeys(char* buffer, char* driverPath)
{
    addKeyValue(buffer, DRIVER_FULL_NAME);
    addKeyValue(buffer, INSTALL_DRIVER, driverPath);
    addKeyValue(buffer, INSTALL_SETUP, driverPath);
    addKeyValue(buffer, INSTALL_API_LEVEL, VALUE_API_LEVEL);
    addKeyValue(buffer, INSTALL_CONNECT_FUN, VALUE_CONNECT_FUN);
    addKeyValue(buffer, INSTALL_FILE_USAGE, VALUE_FILE_USAGE);
    addKeyValue(buffer, INSTALL_DRIVER_VER, VALUE_DRIVER_VER);
    addKeyValue(buffer, INSTALL_SQL_LEVEL, VALUE_SQL_LEVEL);
    addKeyValue(buffer, "");
}

//////////////////////////////////////////////////////////////////////
// Setup Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Setup::Setup(HWND windowHandle, const char* drvr, const char* attr)
{
    hWnd = windowHandle;
    serviceDb = enOpenDb;

    if (drvr) {
        driver = drvr;
    }

    if (attr) {
        attributes = attr;
        dsn = getAttribute(SETUP_DSN);
    }
}

Setup::~Setup()
{}

void Setup::configDsn()
{
    if (!dsn.empty()) {
        readAttributes();
    }
    configureDialog();
}

void Setup::getParameters()
{
    description = getAttribute(SETUP_DESCRIPTION);

    user = getAttribute(SETUP_USER);
    if (user.empty()) {
        user = getAttribute(KEY_DSN_UID);
    }

    password = getAttribute(SETUP_PASSWORD);
    if (password.empty()) {
        password = getAttribute(KEY_DSN_PWD);
    }

    schema = getAttribute(SETUP_SCHEMA);
    if (schema.empty()) {
        schema = getAttribute(KEY_DSN_SCHEMA);
    }
}

bool Setup::addDsn()
{
    getParameters();

    do {
        dbName = getAttribute(SETUP_DBNAME);
        if (!dbName.empty()) {
            break;
        }

        dbName = getAttribute(KEY_DSN_DATABASE);
        if (!dbName.empty()) {
            break;
        }
    } while (false);

    if (serviceDb) {
        CServiceClient services;

        if (!services.initServices()) {
            std::ostringstream text;
            text << "Unable to connect to data source: library '" << DRIVER_NAME << "' failed to load";
            SQLPostInstallerError(ODBC_ERROR_CREATE_DSN_FAILED, text.str().c_str());
            return false;
        }

        if (!services.checkVersion()) {
            std::ostringstream text;
            text << " Unable to load " << DRIVER_NAME << " Library : can't find ver. " << DRIVER_VERSION;
            SQLPostInstallerError(ODBC_ERROR_CREATE_DSN_FAILED, text.str().c_str());
            return false;
        }

        if (!user.empty()) {
            services.putParameterValue("user", user);
        }
        if (!password.empty()) {
            services.putParameterValue("password", password);
        }
        if (!schema.empty()) {
            services.putParameterValue("schema", schema);
        }
        if (!dbName.empty()) {
            services.putParameterValue(SETUP_DBNAME, dbName);
        }
    }

    if (hWnd || dsn.empty()) {
        configureDialog();
    } else if (!SQLWriteDSNToIni(dsn.c_str(), driver.c_str())) {
        MessageBoxInstallerError("Config DSN", NULL);
        return false;
    } else {
        writeAttributes();
    }

    return true;
}

bool Setup::removeDsn()
{
    if (!dsn.empty()) {
        SQLRemoveDSNFromIni(dsn.c_str());
    }

    getParameters();

    if (serviceDb) {
        CServiceClient services;

        if (!services.initServices()) {
            std::ostringstream text;
            text << "Unable to connect to data source: library '" << DRIVER_NAME << "' failed to load";
            SQLPostInstallerError(ODBC_ERROR_CREATE_DSN_FAILED, text.str().c_str());
            return false;
        }

        if (!services.checkVersion()) {
            std::ostringstream text;
            text << "Unable to load " << DRIVER_NAME << " Library : can't find ver. " << DRIVER_VERSION;
            SQLPostInstallerError(ODBC_ERROR_CREATE_DSN_FAILED, text.str().c_str());
            return false;
        }

        if (!user.empty()) {
            services.putParameterValue("user", user);
        }
        if (!password.empty()) {
            services.putParameterValue("password", password);
        }
        if (!schema.empty()) {
            services.putParameterValue("schema", schema);
        }
        if (!dbName.empty()) {
            services.putParameterValue(SETUP_DBNAME, dbName);
        }
        return false;
    }

    return true;
}

std::string Setup::getAttribute(const char* attribute)
{
    const char* p;
    size_t count = strlen(attribute);

    for (p = attributes; *p || *(p+1); ++p) {
        if (p - attributes > 4096) {
            break; // attributes should be finished "\0\0"
        }
        if (*p == *attribute && !strncasecmp(p, attribute, count)) {
            p += count;
            while (*p && (*p == ' ' || *p == '\t')) {
                ++p;
            }
            if (*p == '=') {
                ++p;
                while (*p && (*p == ' ' || *p == '\t')) {
                    ++p;
                }

                const char* q;
                for (q = p; !IS_END_TOKEN(*q); ++q) {}
                return std::string(p, q - p);
            }
        }
        while (!IS_END_TOKEN(*p)) {
            ++p;
        }
    }

    return std::string();
}

bool Setup::configureDialog()
{
    CDsnDialog dialog(hWnd);
    dialog.m_name = dsn;
    dialog.m_description = description;
    dialog.m_database = dbName;
    dialog.m_user = user;
    dialog.m_password = password;
    dialog.m_schema = schema;

    do {
        intptr_t ret = dialog.DoModal();
        if (ret != IDOK) {
            return false;
        }

        if (SQLValidDSN(dialog.m_name.c_str())) {
            break;
        }

        if (!MessageBoxInstallerError("Config DSN", NULL)) {
            MessageBox(NULL,
                       "Invalid characters are included \
                        in the data source name: []{}(),;?*=!@\\",
                       DRIVER_NAME,
                       MB_ICONSTOP | MB_OK);
        }
    } while (true);

    if (dsn != dialog.m_name) {
        SQLRemoveDSNFromIni(dsn.c_str());
    }

    dsn = dialog.m_name;
    description = dialog.m_description;
    dbName = dialog.m_database;
    user = dialog.m_user;
    password = dialog.m_password;
    schema = dialog.m_schema;

    if (!SQLWriteDSNToIni(dialog.m_name.c_str(), driver.c_str())) {
        MessageBoxInstallerError("Config DSN", NULL);
        return false;
    } else {
        writeAttributes();
    }

    return true;
}

void Setup::writeAttributes()
{
    writeAttribute(SETUP_DESCRIPTION, description);
    writeAttribute(SETUP_DBNAME, dbName);
    writeAttribute(SETUP_USER, user);
    writeAttribute(SETUP_PASSWORD, password);
    writeAttribute(SETUP_SCHEMA, schema);
}

void Setup::readAttributes()
{
    description = readAttribute(SETUP_DESCRIPTION);
    dbName = readAttribute(SETUP_DBNAME);
    user = readAttribute(SETUP_USER);
    password = readAttribute(SETUP_PASSWORD);
    schema = readAttribute(SETUP_SCHEMA);
}

void Setup::writeAttribute(const char* attribute, const std::string& value)
{
    SQLWritePrivateProfileString(dsn.c_str(), attribute, value.c_str(), "ODBC.INI");
}

std::string Setup::readAttribute(const char* attribute)
{
    char buffer[256];

    int ret = SQLGetPrivateProfileString(dsn.c_str(), attribute, "", buffer, sizeof(buffer), "ODBC.INI");

    return std::string(buffer, ret);
}

// static
std::string Setup::getInstallerError()
{
    char    buf[SQL_MAX_MESSAGE_LENGTH];
    DWORD   errorCode;

    if (SQLInstallerError(1, &errorCode, buf,  sizeof(buf)-1, 0) == SQL_NO_DATA) {
        return std::string();
    }

    std::ostringstream msg;
    msg << errorCode << "/" << buf;
    return msg.str();
}
