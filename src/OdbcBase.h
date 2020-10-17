/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <stddef.h>

#if defined(_WIN32)
# include <windows.h>
# define strcasecmp     _stricmp
# define strncasecmp    _strnicmp
#else
# include <stdio.h>
# define OutputDebugString(string) fputs(string, stdout)
#endif

extern "C" {
#include <sql.h>
#include <sqlext.h>
}

#ifndef _ASSERT
# define _ASSERT(what)
#endif

#if !defined(__GNUC__) && !defined(__attribute__)
# define __attribute__(_x)
#endif

#if defined(NUOODBC_DEBUG)
# define DBG(_m) nuoodbc_debug _m
void nuoodbc_debug(const char* msg, ...);
#else
# define DBG(_m)
#endif
