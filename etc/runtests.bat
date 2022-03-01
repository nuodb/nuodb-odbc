@echo off
::
:: (C) Copyright NuoDB, Inc. 2020  All Rights Reserved.
::
:: This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
:: See the LICENSE file provided with this software.
::
:: Start a test DB for use with NuoODBCTest
:: then run the unit tests

setlocal

set instdir=%~1
set testexe=%~2
if "%testexe%" == "" (
    echo usage: %0 ^<installdir^> ^<testexe^>
    exit /b 1
)

set "dbname=NuoODBCTestDB"
set "dbauser=dba"
set "dbapwd=dba"

:: ---- Setup

set "tmpdir=%TEMP%\%DBNAME%_tmp"
set "ardir=%TEMP%\%DBNAME%_ar"
set "dbstarted=false"

if not exist "%instdir%\bin\NuoODBC.dll" (
    echo NuoDB ODBC driver bin\NuoODBC.dll not found in %instdir%
    goto FAIL
)
if not exist "%instdir%\etc\nuoodbc.bat" (
    echo Registration script etc\nuoodbc.bat not found in %instdir%
    goto FAIL
)
if not exist %testexe% (
    echo Test %testexe% does not exist
    goto FAIL
)

if exist "%tmpdir%" rmdir /S /Q "%tmpdir%"
mkdir "%tmpdir%"

call :CHECKAP
if errorlevel 1 goto FAIL

call :DELETEDB
if errorlevel 1 goto FAIL

:: ----- Manage DB

echo Installing NuoDB ODBC driver ...
cmd /s /c "%instdir%\etc\nuoodbc.bat install"
if errorlevel 1 goto FAIL

echo Creating database %dbname% ...

:: Create the database
rmdir /S /Q "%ardir%"
cmd /s /c "nuocmd.bat create archive --server-id nuoadmin-0 --db-name %dbname% --archive-path "%ardir%""
if errorlevel 1 (
    echo %dbname%: Failed to create an archive at %ardir%
    goto FAIL
)

cmd /s /c "nuocmd.bat create database --db-name %dbname% --te-server-ids nuoadmin-0 --dba-user %dbauser% --dba-password %dbapwd%"
if errorlevel 1 (
    echo %dbname%: Failed to create database
    goto FAIL
)

cmd /s /c "nuocmd.bat check database --db-name %dbname% --check-running --timeout 30 --num-processes 2"
if errorlevel 1 (
    echo %dbname%: Failed to go RUNNING
    goto FAIL
)

set "dbstarted=true"

cmd /s /c "nuocmd.bat show domain"

echo Environment:
echo   Temp dir:     %tmpdir%
echo   Archive dir:  %ardir%
echo   DB Name:      %dbname%
echo   DBA User:     %dbauser%
echo   DBA Password: %dbapwd%

set "NUOODBC_TEST_TEMP=%tmpdir%"
set "NUODB_DBNAME=%dbname%"
set "NUODB_USER=%dbauser%"
set "NUODB_PASSWORD=%dbapwd%"

:: We may not be able to connect immediately due to bootstrap, esp on Windows

set ctr=0
:startloop
echo "select 1 from dual;" | nuosql "%dbname%@localhost" --user "%dbauser%" --password "%dbapwd%" >nul 2>&1
if %ERRORLEVEL% == 0 goto ready
set /A ctr=%ctr%+1
if %ctr% GTR 10 (
    echo Timed out waiting to connect to database %dbname%
    goto FAIL
)
ping -n 2 127.0.0.1 >nul
goto startloop

:: Poor-man's arg forwarding
ready:
%testexe% %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 (
    echo Unit test %testexe% failed
    goto FAIL
)

:: Testing succeeded so clean up

call :DELETEDB
if errorlevel 1 goto FAIL

cmd /s /c "%instdir%\etc\nuoodbc.bat uninstall"
if errorlevel 1 goto FAIL

rmdir /S /Q "%tmpdir%"

echo SUCCESS
exit /b 0

:: ---- Check the AP

:: On Linux we start it if necessary but thst's too hard to manage on Windows
:: with all the different ways it could run, needing admin privileges, etc. so
:: we just require it be started before we run the tests.

:CHECKAP
where /Q nuocmd.bat
if errorlevel 1 (
    echo Cannot locate nuocmd.bat: please set PATH correctly.
    exit /b 1
)

cmd /s /c "nuocmd.bat get servers" >nul
if errorlevel 1 (
    echo Cannot connect to the NuoDB Admin Process
    echo Make sure that 'nuocmd.bat get servers' succeeds
    exit /b 1
)
goto :EOF

:: ---- Delete the test database

:DELETEDB
set "out=%tmpdir%\deldb.out"

:: If there's no archive then there is no database
cmd /s /c "nuocmd.bat show archives --db-name %dbname% --archive-format "{id}"" >"%tmpdir%\archive.out" 2>nul
if errorlevel 1 exit /b 0
set /p archiveid= < "%tmpdir%\archive.out"

:: If the DB doesn't exist nothing to do
cmd /s /c "nuocmd.bat show database --db-name %dbname%" >"%out%" 2>&1
if errorlevel 1 exit /b 0

:: TOMBSTONE means it is deleted
findstr /C:"state = TOMBSTONE" "%out%" >nul
if not errorlevel 1 goto delar

findstr /C:"state = NOT_RUNNING" "%out%" >nul
if not errorlevel 1 goto deldb

:: DB is running so shut it down
cmd /s /c "nuocmd.bat shutdown database --db-name %dbname%"
if errorlevel 1 (
    echo %dbname%: Cannot shut down running database
    exit /b 1
)

:: Wait for all engines to exit
cmd /s /c "nuocmd.bat check database --db-name %dbname% --num-processes 0 --timeout 30"
if errorlevel 1  (
    echo %dbname%: Timed out waiting to shut down
    exit /b 1
)

:deldb
echo Deleting database %dbname%
cmd /s /c "nuocmd.bat delete database --db-name %dbname%"
if errorlevel 1 (
    echo %dbname%: Failed to delete database
    exit /b 1
)

:delar
if "%archiveid%" == "" (
    rmdir /S /Q "%ardir%"
    exit /b 0
)

echo Deleting archive %archiveid%
cmd /s /c "nuocmd.bat delete archive --archive-id "%archiveid%" --purge"
if errorlevel 1 (
    echo Cannot delete archive ID %archiveid%
    exit /b 1
)
rmdir /S /Q "%ardir%"
exit /b 0

:: ---- Fail the script

:FAIL
echo *** FAILED 1>&2
if "%dbstarted%" == "true" (
    echo Leaving DB running.  Test output in %tmpdir%
    cmd /s /c "nuocmd.bat show domain" 2>nul
)
echo on
exit /b 1
