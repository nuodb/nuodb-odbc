# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

###
### Find the ODBC libraries
###

add_library(OdbcLib INTERFACE)
add_library(OdbcTestLib INTERFACE)

if(WIN32)
    target_link_libraries(OdbcLib INTERFACE
        ODBCCP32 Version Comctl32 legacy_stdio_definitions)
    target_link_libraries(OdbcTestLib INTERFACE
        ODBCCP32 ODBC32 Version Comctl32)

else()
    if(ODBC_INCLUDE)
        target_include_directories(OdbcLib INTERFACE "${ODBC_INCLUDE}")
        target_include_directories(OdbcTestLib INTERFACE "${ODBC_INCLUDE}")
    endif()

    if(ODBC_LIB)
        target_link_directories(OdbcLib INTERFACE "${ODBC_LIB}")
        target_link_directories(OdbcTestLib INTERFACE "${ODBC_LIB}")
    endif()

    target_link_libraries(OdbcLib INTERFACE odbc odbcinst)
    target_link_libraries(OdbcTestLib INTERFACE odbc odbcinst)
endif()
