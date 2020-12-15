# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

###
### Unit Tests
###

add_executable(NuoODBCTest
    ODBCTestBase.h
    ODBCTransactionTestRequiresChorus.cpp
    ODBCMetadataTestRequiresChorus.cpp
    ODBCEscapeTestRequiresChorus.cpp)

set_target_properties(NuoODBCTest PROPERTIES
    CXX_STANDARD 17)

# The ODBC driver is not linked, it's loaded separately, so we don't depend on
# it but it must be built before the tests can run!
add_dependencies(NuoODBCTest NuoODBC)

target_include_directories(NuoODBCTest PRIVATE
    ${CMAKE_BINARY_DIR}/src)

target_link_libraries(NuoODBCTest PRIVATE
    OdbcTestLib gtestlib Threads::Threads)
