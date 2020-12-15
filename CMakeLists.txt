# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

set(NUOODBC_COPYRIGHT "(C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.")

cmake_minimum_required(VERSION 3.12)

project(NuoODBC CXX)

set(NUOODBC_PRODUCT "NuoDB ODBC Driver")
set(NUOODBC_VERSION 2 0 0)

include(FindThreads)
include(CTest)

include(CMake/Setup.cmake)
include(CMake/NuoDBClient.cmake)
include(CMake/Odbc.cmake)

add_subdirectory(src)

if(BUILD_TESTING)
    include(CMake/GTest.cmake)
    add_subdirectory(test)
endif()

if(WIN32)
    install(PROGRAMS etc/nuoodbc.bat TYPE SYSCONF)
endif()

message(STATUS "")
message(STATUS "CMake version:      ${CMAKE_VERSION}")
message(STATUS "CMake generator:    ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM}")
message(STATUS "C++ Compiler ID:    ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Compiler Ver:   ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Target System:      ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message(STATUS "")
message(STATUS "Install directory:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "NuoDB C++ Client:   ${NUOCLIENT_PREFIX}")
if(ODBC_INCLUDE)
    message(STATUS "ODBC Library -I:    ${ODBC_INCLUDE}")
endif()
if(ODBC_LIB)
    message(STATUS "ODBC Library -L:    ${ODBC_LIB}")
endif()
if(BUILD_TESTING)
    message(STATUS "GoogleTest source:  ${GTEST_BASE}")
endif()
