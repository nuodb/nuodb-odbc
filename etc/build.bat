@echo off
:: (C) Copyright 2022 NuoDB, Inc.  All Rights Reserved.
::
:: This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
:: See the LICENSE file provided with this software.

setlocal

pushd %~dp0..
set "BASE=%CD%"
popd

if "%1" == "-h" goto HELP
if "%1" == "--help" goto HELP
if not "%1" == "" (
    echo usage: build.bat
    exit /b 1
)

set "download=https://ce-downloads.nuohub.org"
set "arch=win64"

set "tmpdir=%BUILDTEMP%"
if "%tmpdir%" == "" set "tmpdir=%TEMP%\nuobldodbc"

if exist "%tmpdir%" rmdir /s /q "%tmpdir%"
mkdir "%tmpdir%"
if errorlevel 1 goto FAIL

if not "%NUODB_HOME%" == "" goto build
echo .. Retrieving NuoDB Server version list

cmd /s /c "curl -s -o "%tmpdir%\ver.txt" "%download%/supportedversions.txt""
if errorlevel 1 goto FAIL

set ver=
for /f %%a in (%tmpdir%\ver.txt) do set "ver=%%a"

if not "%SERVER_PKG%" == "" goto unpack

echo .. Retrieving NuoDB Server version %ver%
cmd /s /c "curl -s -o "%tmpdir%\nuodb.zip" "%download%/nuodb-%ver%.%arch%.zip""
if errorlevel 1 goto FAIL
set "SERVER_PKG=%tmpdir%\nuodb.zip"

:unpack
echo .. Unpacking NuoDB Server version %ver%
cmd /s /c "unzip -q -d "%tmpdir%" "%SERVER_PKG%""
if errorlevel 1 goto FAIL

rename "%tmpdir%\nuodb-%ver%.%arch%" nuodb
set "NUODB_HOME=%tmpdir%\nuodb"

:build
cmd /s /c ""%NUODB_HOME%\bin\nuodb" --version" >nul 2>&1
if errorlevel 1 (
    echo Cannot run %NUODB_HOME%\bin\nuodb
    goto FAIL
)

for /f "delims=" %%v in ('%NUODB_HOME%\bin\nuodb --version') do set "fullver=%%v"
if errorlevel 1 goto FAIL

echo .. Building against %fullver%

if exist "%tmpdir%\obj" rmdir /s /q "%tmpdir%\obj"
if exist "%tmpdir%\dist" rmdir /s /q "%tmpdir%\dist"
mkdir "%tmpdir%\obj"
mkdir "%tmpdir%\dist"

if "%BUILD_TYPE%" == "" set "BUILD_TYPE=RelWithDebInfo"

pushd "%tmpdir%\obj"
cmd /s /c "cmake "%BASE%" "-DNUODB_HOME=%NUODB_HOME%" "-DCMAKE_INSTALL_PREFIX=%tmpdir%\dist" %CFG_ARGS%"
if errorlevel 1 (
    popd
    goto FAIL
)

cmd /s /c "cmake --build . --target install --config "%BUILD_TYPE%""
if errorlevel 1 (
    popd
    goto FAIL
)

set odbcver=
for /f %%a in (%tmpdir%\obj\etc\version.txt) do set "odbcver=%%a"

set "dirnm=nuodbodbc-%odbcver%.%arch%"
echo .. Creating %tmpdir%\%dirnm%.zip
cd ..
rename "dist" "%dirnm%"
7z a -bd -tzip "%dirnm%.zip" "%dirnm%"
if errorlevel 1 (
    popd
    goto FAIL
)

popd
echo Build complete:
echo.    Directory: %tmpdir%\%dirnm%
echo.    Package:   %tmpdir%\%dirnm%.zip
exit /b 0

:HELP
echo Build the NuoDB ODBC driver.
echo.
echo By default, download the latest NuoDB Server package to retrieve the
echo NuoDB C++ driver and build against it.
echo.
echo Behaviors can be modified via various environment variables:
echo.
echo BUILDTEMP    : Temporary directory to use for builds.
echo.               If not set, %%TEMP%%\nuobldodbc is used.
echo.
echo NUODB_HOME   : An existing NuoDB Server installation.
echo.               If not set, download the latest and unpack it.
echo.
echo SERVER_PKG   : An already-downloaded NuoDB Server package.
echo.               If not set, download the package.
echo.
echo BUILD_TYPE   : Type of build.
echo.               Default is RelWithDebInfo.
echo.
echo CFG_ARGS     : Extra arguments to pass to cmake configuration.
echo.               Default is empty.
echo.
echo BLD_ARGS     : Extra arguments to pass to cmake build.
echo.               Default is empty.
echo.
echo The results will be installed into %%BUILDTEMP%%.
exit /b 0

:FAIL
echo *** FAILED
exit /b 1
