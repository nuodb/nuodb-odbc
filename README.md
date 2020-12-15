# NuoDB - ODBC

[![Test Results](https://travis-ci.org/nuodb/nuodb-odbc.svg?branch=main)](https://travis-ci.org/nuodb/nuodb-python)

This repository provides the NuoDB_ client library for ODBC.  It is supported
both on Microsoft Windows and on GNU/Linux (using unixODBC).

This driver uses the NuoDB C++ Driver to communicate with NuoDB.

# License

NuoDB ODBC is licensed under the [MIT
License](https://github.com/nuodb/nuodb-odbc/blob/main/LICENSE)
*EXCEPT WHERE OTHERWISE SPECIFIED IN THE SOURCE*.

# Runtime Requirements

## On GNU/Linux systems:

You will need to install the unixODBC library before you can run the driver.

On Debian/Ubuntu:

```shell
  sudo apt install libodbc1
```

On Red Hat/CentOS:

```shell
  sudo yum install libodbc1
```

## On Windows systems:

Windows already provides the ODBC infrastructure.

In order to register (and deregister) the driver with the Windows ODBC system,
you can use the `.\etc\nuoodbc.bat` file to update the Windows registry.  You
must have administrator privileges to install/uninstall.

To install the driver use:

```batch
  .\etc\nuoodbc.bat install [<dll>]
```

If `<dll>` is given it should be the fully-qualified pathname to the NuoODBC
Driver DLL file.  If not given it will be assumed to be `..\bin\NuoODBC.dll`
from the location of the `nuoodbc.bat` file.

To uninstall the driver use:

```batch
  .\etc\nuoodbc.bat uninstall
```

And to check the status of the currently-installed NuoODBC driver (if any)
use:

```batch
  .\etc\nuoodbc.bat status
```

# Building the Driver

The NuoDB ODBC driver can be built on GNU/Linux and on Windows.

You will need NuoDB 4.1.1 or above to build the driver.  You can build using
either the
[NuoDB database package](https://www.nuodb.com/dev-center/community-edition-download)
or with the [NuoDB client package](https://github.com/nuodb/nuodb-client).

## Install Prerequisites

To build the NuoDB ODBC driver you will need some prerequisites installed on
your system, that are not needed to use the driver.

### GNU/Linux Prerequisites

To build the driver on GNU/Linux you will need these packages:

On Debian/Ubuntu systems:

```shell
  sudo apt install build-essential cmake unixodbc-dev
```

On Red Hat/CentOS systems:

```shell
  sudo yum install build-essential cmake unixodbc-dev
```

If you want to build the unit tests, you may install the `googletest` package
from your package manager.  If it is found on the system then the build will
use it: if it is not found then the GoogleTest source will be downloaded from
GitHub.

On Debian/Ubuntu systems:

```shell
  sudo apt install googletest
```

### Windows Prerequisites

To build the driver you will need to install Microsoft Visual Studio or other
C++ compiler.  The build has been tested with Visual Studio 16 (2019).

You must install the ATL and MFC modules in Visual Studio.

You will need to install the Windows Software Development Kit.

You will also need to install [CMake](https://cmake.org/download/).

## Configure the Driver

Use `cmake` to configure the driver.  We test using a separate build
directory.  On GNU/Linux you might use:

```shell
  cd nuodb-odbc
  mkdir obj
  cd obj
  cmake .. -DCMAKE_INSTALL_PREFIX=dist [...]
```

On Windows you must choose 64bit.  You might use:

```batch
  cd nuodb-odbc
  mkdir obj
  cd obj
  cmake -A x64 -G "Visual Studio 16 2019" .. -DCMAKE_INSTALL_PREFIX=dist [...]
```

where `[...]` are any variable settings as below.

* `-DNUOCLIENT_HOME=<dir>` : (default `$NUOCLIENT_HOME`) Path to the NuoDB
  Client installation.
* `-DNUODB_HOME=<dir>` : (default `$NUODB_HOME`) Path to the NuoDB Database
  installation.

If both are found then `NUOCLIENT_HOME` will be preferred.  If neither is set
then on GNU/Linux `/opt/nuodb` will be used and on Windows `C:\Program
Files\NuoDB` will be used.  If none of these exist `cmake` will fail.

* `-DBUILD_TESTING=<bool>` : (default `ON`) Build the unit tests or not.
* `-DGOOGLETEST_SRC=<dir>` : (default `/usr/src/googletest`) Path to a local
  GoogleTest installation.  If not found and testing is enabled, the source
  for GoogleTest will be downloaded from GitHub.
* `-DCMAKE_INSTALL_PREFIX=<dir>` : (default `/usr/local` on GNU/Linux or
  `C:\Program Files\NuoODBC` on Windows) Where to install the driver.

These extra configure options are available on GNU/Linux:

* `-DCMAKE_BUILD_TYPE=<type>` : (default `RelWithDebInfo`) The build type;
  choose one of `Debug`, `RelWithDebInfo`, `Release`, `MinSizeRel`.
* `-DODBC_INCLUDE=<dir>` : (no default) Use `<dir>` to locate unixODBC header
  files.  By default they are searched for in standard system locations.
* `-DODBC_LIB=<dir>` : (no default) Use `<dir>` to locate unixODBC libraries.
  By default they are searched for in standard system locations.

## Build the Driver

After configuration has completed you can build the driver and/or unit tests.
On GNU/Linux use:

```shell
  cmake --build .
```

Or, run `make` directly.

On Windows use:

```batch
  cmake --build . --config RelWithDebInfo
```

If you want to install as well you can add the `--target install` option.
Note, you must install if you want to run the unit tests.

*NOTE* Windows installation only copies files: it does not register the
driver; see _Runtime Requirements_ above.

# Running the Tests

To run the unit tests you must be able to connect to an existing NuoDB Admin
Process via the `nuocmd` script, which must be on your PATH, so that the test
scripts can create a test database.

The test database will be named `NuoODBCTestDB`.  If a database named
`NuoODBCTestDB` exists **it will be deleted!** by the test scripts, so beware.

## Testing on GNU/Linux

To run the tests on GNU/Linux use:

```shell
  ./etc/runtests.sh <instdir> <testexe>
```

where `<instdir>` is the directory used with `CMAKE_INSTALL_PREFIX` during the
configuration (e.g., `obj/dist`), and `<testexe>` is the unit test to be run
(e.g., `obj/test/NuoODBCTest.exe`).

## Testing on Windows

The test script will use the `nuoodbc.bat` script to install the newly-built
driver into the Windows registry (see _Runtime Requirements_ above).  This
will **replace** any existing NuoODBC Driver configuration in the registry.

To run the tests on GNU/Linux from the `nuodb-odbc` source directory, use:

```batch
  .\etc\runtests.bat <instdir> <testexe>
```

where `<instdir>` is the directory used with `CMAKE_INSTALL_PREFIX` during the
configuration (e.g. `obj\dist`), and `<testexe>` is the unit test to be run
(e.g., `obj\test\RelWithDebInfo\NuoODBCTest.exe`).

# Resources

* [NuoDB Documentation](https://doc.nuodb.com/Latest/Default.htm)
* [NuoDB Database Download](https://www.nuodb.com/dev-center/community-edition-download)
* [NuoDB Client Download](https://github.com/nuodb/nuodb-client/releases)
