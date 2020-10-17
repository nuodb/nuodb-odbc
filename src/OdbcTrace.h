/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <cstdarg>

#include <string>
#include <sstream>
#include <algorithm>

#include "OdbcBase.h"
#include "OdbcObject.h"

//#define DEBUG 1

#ifdef DEBUG
# define TRACE(msg)             trace(msg)
# define TRACERET(msg, retcode) trace(msg, retcode);
#else
# define TRACE(msg)
# define TRACERET(msg, retcode)
#endif

#ifdef _WIN64
# define SQLULEN_FMT    "%I64u"
# define SQLLEN_FMT     "%I64d"
#else
# define SQLULEN_FMT    "%lu"
# define SQLLEN_FMT     "%ld"
#endif

inline void notYetImplemented(const char* msg)
{
    OutputDebugString("NotYetImplemented: ");
    OutputDebugString(msg);
    OutputDebugString("\n");
}

inline RETCODE notYetImplemented(OdbcObject* object, const char* msg)
{
    notYetImplemented(msg);

    return object->sqlReturn(SQL_ERROR, "HY000", msg);
}

inline void trace(const char* message)
{
    OutputDebugString(message);
    OutputDebugString("\n");
}

inline void trace(const char* message, RETCODE result)
{
    std::ostringstream msg;
    msg << message << " returns " << result << "\n";
    OutputDebugString(msg.str().c_str());
}

inline std::string formatString(const char* pattern, ...)
{
    char buf[16 * 1024];
    va_list args;
    va_start(args, pattern);
    int retval = vsnprintf(buf, sizeof(buf), pattern, args);
    va_end(args);

    if (retval < 0) {
        return std::string();
    }
    return std::string(buf, std::min<size_t>(sizeof(buf), (size_t)retval));
}
