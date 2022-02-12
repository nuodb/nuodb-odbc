# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

include(ExternalProject)

if(GOOGLETEST_SRC)
    set(_gtest_src "${GOOGLETEST_SRC}")
else()
    set(_gtest_src /usr/src/googletest)
endif()

set(_gtest_args
    -DBUILD_GMOCK=OFF
    -DINSTALL_GTEST=OFF
    -Dgtest_force_shared_crt=ON)

if(NOT "${CMAKE_C_COMPILER}" STREQUAL "")
    list(APPEND _gtest_args
        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
        "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
endif()

if(NOT "${CMAKE_CXX_COMPILER}" STREQUAL "")
    list(APPEND _gtest_args
        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
endif()

if(EXISTS "${_gtest_src}/CMakeLists.txt")
    set(GTEST_BASE ${_gtest_src})
    ExternalProject_Add(googletest
        PREFIX googletest
        SOURCE_DIR "${GTEST_BASE}"
        CMAKE_ARGS ${_gtest_args}
        INSTALL_COMMAND "")
else()
    set(GTEST_BASE "https://github.com/google/googletest.git")
    ExternalProject_Add(googletest
        GIT_REPOSITORY "${GTEST_BASE}"
        GIT_TAG "main"
        CMAKE_ARGS ${_gtest_args}
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON)
endif()

ExternalProject_Get_Property(googletest SOURCE_DIR BINARY_DIR)

add_library(gtestlib INTERFACE)
add_dependencies(gtestlib googletest)

target_include_directories(gtestlib INTERFACE "${SOURCE_DIR}/googletest/include")

target_link_directories(gtestlib INTERFACE "${BINARY_DIR}/lib")

# Building googletest in Visual Studio in Debug mode generates "d" libraries
target_link_libraries(gtestlib INTERFACE
    $<IF:$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>,
        gtest_maind gtestd,
        gtest_main gtest>)
