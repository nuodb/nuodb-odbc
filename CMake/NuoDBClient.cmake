# (C) Copyright NuoDB Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

###
### Find NuoDB Client package
###

# Why is dealing with CMake variables and the cache so hard?
# I just want the most obvious behavior: if a variable is set on the command
# line it should take precedence over any local setting or previous cache
# setting, and if it's a cached variable the new value should be remembered in
# the cache.  If the variable is not set on the command line then the cached
# value (if any) should be used.  Isn't that the way everyone would expect it
# to work?  I must be missing something really basic.

set(_nuoclient "${NUOCLIENT_HOME}")
if(_nuoclient STREQUAL "")
    set(_nuoclient "${NUODB_HOME}")
endif()
if(_nuoclient STREQUAL "")
    set(_nuoclient "$ENV{NUOCLIENT_HOME}")
endif()
if(_nuoclient STREQUAL "")
    set(_nuoclient "$ENV{NUODB_HOME}")
endif()
if(_nuoclient STREQUAL "")
    if(WIN32)
        set(_default "C:/Program Files/NuoDB")
    else()
        set(_default "/opt/nuodb")
    endif()
    if(IS_DIRECTORY "${_default}")
        set(_nuoclient "${_default}")
    endif()
endif()
if(_nuoclient STREQUAL "" AND NUOCLIENT_PREFIX STREQUAL "")
    message(FATAL_ERROR "NuoDB client not found: set NUOCLIENT_HOME or NUODB_HOME")
endif()

if(_nuoclient)
    set(NUOCLIENT_PREFIX "${_nuoclient}" CACHE PATH "NuoDB client prefix" FORCE)
else()
    set(NUOCLIENT_PREFIX "${_nuoclient}" CACHE PATH "NuoDB client prefix")
endif()

add_library(NuoClient INTERFACE)

target_include_directories(NuoClient INTERFACE "${NUOCLIENT_PREFIX}/include")
target_link_directories(NuoClient INTERFACE "${NUOCLIENT_PREFIX}/${NUODB_LIB}")
target_link_libraries(NuoClient INTERFACE NuoRemote)
