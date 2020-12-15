@echo off
:: -- Manage NuoDB ODBC driver from the command line
:: On Windows we can only have one NuoDB ODBC driver installed at a time
:: (C) Copyright 2019-2020 NuoDB, Inc.  All Rights Reserved.
::
:: This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
:: See the LICENSE file provided with this software.

setlocal

pushd %~dp0..
set "odbchome=%CD%"
popd

set cmd=%1

set "odbcname=NuoDB ODBC Driver"
set "odbcpre=HKLM\Software\ODBC\ODBCINST.INI"
set "odbcbase=%odbcpre%\%odbcname%"

if "%cmd%" == "install" goto installodbc
if "%cmd%" == "uninstall" goto uninstallodbc
if "%cmd%" == "status" goto statusodbc
echo Unknown command: %cmd%
goto FAIL

:installodbc
reg query "%odbcpre%\ODBC Drivers" /v "%odbcname%" >nul 2>&1
if not errorlevel 1 (
    echo "%odbcname% is already installed."
    goto FAIL
)

set "dll=%2"
if "%dll%" == "" set "dll=%odbchome%\bin\NuoODBC.dll"

if not exist "%dll%" (
    echo NuoDB ODBC Driver %dll% does not exist
    goto FAIL
)

echo Setting %odbcname% to use %dll%

reg add "%odbcpre%\ODBC Drivers" /v "%odbcname%" /t REG_SZ /d "Installed" /f >nul

reg add "%odbcbase%" /v Driver              /t REG_SZ /d "%dll%" /f >nul
reg add "%odbcbase%" /v Setup               /t REG_SZ /d "%dll%" /f >nul
reg add "%odbcbase%" /v UsageCount          /t REG_DWORD /d 0x00000001 /f >nul
reg add "%odbcbase%" /v SQLLevel            /t REG_SZ /d "1" /f >nul
reg add "%odbcbase%" /v FileUsage           /t REG_SZ /d "0" /f >nul
reg add "%odbcbase%" /v DriverODBCVer       /t REG_SZ /d "03.50" /f >nul
reg add "%odbcbase%" /v ConnectionFunctions /t REG_SZ /d "YYY" /f >nul
reg add "%odbcbase%" /v APILevel            /t REG_SZ /d "1" /f >nul
reg add "%odbcbase%" /v CPTimeout           /t REG_SZ /d "60" /f >nul
goto COMPLETE

:uninstallodbc
echo Removing NuoDB ODBC Driver configuration
reg delete "%odbcpre%\ODBC Drivers" /v "%odbcname%" /f >nul 2>&1
reg delete "%odbcbase%" /f >nul 2>&1
goto COMPLETE

:statusodbc
reg query "%odbcbase%" /v Driver >nul 2>&1
if ERRORLEVEL 1 (
    set "ODBCDRIVER=Not Installed"
) else (
    for /f "tokens=1,2*" %%a in ('reg query "%odbcbase%" /v Driver') do set "ODBCDRIVER=%%c"
)
echo NuoDB ODBC Driver: %ODBCDRIVER%
goto COMPLETE

:COMPLETE
exit /b 0

:FAIL
echo *** FAILED
exit /b 1
