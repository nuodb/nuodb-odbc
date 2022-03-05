# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

# For generators that require a built type at configure time, use a default of
# RelWithDebInfo if not specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo"
        CACHE STRING "The type of build." FORCE)
endif()

# NuoDB uses lib64 on GNU/Linux
if(WIN32)
    set(NUODB_LIB "lib")
else()
    set(NUODB_LIB "lib64")
endif()

if(NOT NUOODBC_REVISION_ID)
    # If it's not set already, get the current commit
    # We specifically do NOT want to cache this value in this case, so that
    # it's re-computed every time we run cmake.
    execute_process(COMMAND git rev-parse --short=10 HEAD
        OUTPUT_VARIABLE NUOODBC_REVISION_ID
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
else()
    # If it's set already, it must be from the command line (or cached from a
    # previous command line override) so cache it now.
    set(NUOODBC_REVISION_ID ${NUOODBC_REVISION_ID}
        CACHE STRING "NuoDB ODBC Driver revision ID")
endif()

list(GET NUOODBC_VERSION 0 NUOODBC_VERSION_MAJOR)
list(GET NUOODBC_VERSION 1 NUOODBC_VERSION_MINOR)
list(GET NUOODBC_VERSION 2 NUOODBC_VERSION_PATCH)

set(NUOODBC_BUILD_KEY "${NUOODBC_VERSION_MAJOR}${NUOODBC_VERSION_MINOR}${NUOODBC_VERSION_PATCH}")

configure_file(etc/version.txt.in etc/version.txt)
configure_file(src/ProductVersion.h.in src/ProductVersion.h)
