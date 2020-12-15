# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

###
### NuoODBC: ODBC driver library
###

add_library(NuoODBC SHARED
    Bindings.h
    DescRecord.h
    GetDataTypeFilter.cpp
    GetDataTypeFilter.h
    GetMapper.cpp
    GetMapper.h
    InfoItems.h
    Main.cpp
    OdbcBase.h
    OdbcConnection.cpp
    OdbcConnection.h
    OdbcDesc.cpp
    OdbcDesc.h
    OdbcEnv.cpp
    OdbcEnv.h
    OdbcError.cpp
    OdbcError.h
    OdbcObject.cpp
    OdbcObject.h
    OdbcStatement.cpp
    OdbcStatement.h
    OdbcTrace.h
    OdbcTypeMapper.cpp
    OdbcTypeMapper.h
    ResultSetFilter.h
    ResultSetMapper.cpp
    ResultSetMapper.h
    SetupAttributes.h

    $<$<BOOL:${WIN32}>:
        Win/NuoODBC.def
        Win/CommonUtil.cpp
        Win/CommonUtil.h
        Win/DsnDialog.cpp
        Win/DsnDialog.h
        Win/OdbcSetup.cpp
        Win/OdbcSetup.h
        Win/OdbcSetup.rc
        Win/ServiceClient.cpp
        Win/ServiceClient.h
        Win/ServiceManager.cpp
        Win/ServiceManager.h
        Win/Setup.cpp
        Win/Setup.h
        Win/Version.rc2
        Win/resource.h
        >
)

set_target_properties(NuoODBC PROPERTIES
    CXX_STANDARD 17
    INSTALL_RPATH "\$ORIGIN")

target_include_directories(NuoODBC PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(NuoODBC PRIVATE
    $<$<CONFIG:Debug>:
        NUOODBC_DEBUG>
    $<$<BOOL:${WIN32}>:
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        >
    )

target_compile_options(NuoODBC PRIVATE
    $<$<BOOL:${CMAKE_COMPILER_IS_GNUCC}>:
        -Wall -Werror
        -Wformat-security
        -Wtype-limits
        -Wunused-but-set-parameter
        -Wlogical-op
        -Wignored-qualifiers
        -Wformat-signedness
        -Wduplicated-cond
        -Wpointer-arith
        -Wsuggest-attribute=pure
        -Wsuggest-attribute=const
        -Wsuggest-attribute=noreturn
        >
    )

target_link_libraries(NuoODBC PRIVATE OdbcLib NuoClient Threads::Threads)

if(WIN32)
    install(TARGETS NuoODBC)

    # We need the DLLs as well
    file(GLOB dlls "${NUOCLIENT_PREFIX}/bin/*.dll")
    install(PROGRAMS ${dlls} TYPE BIN)
else()
    # NuoDB puts libraries in lib64, not lib
    install(TARGETS NuoODBC DESTINATION ${NUODB_LIB})

    # We need the shared libs as well
    file(GLOB sos "${NUOCLIENT_PREFIX}/${NUODB_LIB}/*.so*")
    install(PROGRAMS ${sos} DESTINATION ${NUODB_LIB})
endif()
