/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "OdbcTypeMapper.h"

#include <NuoRemote/ResultSetMetaData.h>


short OdbcTypeMapper::getShort(short value)
{
    return (short)mapType(value);
}

int32_t OdbcTypeMapper::getInt(int32_t value)
{
    return (int32_t)mapType(value);
}

int64_t OdbcTypeMapper::getLong(int64_t value)
{
    return (int64_t)mapType((int)value);
}

// static
NuoDB::SqlType OdbcTypeMapper::mapType(int in)
{
    // map the NuoDB type to an ODBC type
    switch (in) {
        case (int)NuoDB::NUOSQL_BLOB:
            return NuoDB::NUOSQL_LONGVARBINARY;

        case (int)NuoDB::NUOSQL_CLOB:
            return NuoDB::NUOSQL_LONGVARCHAR;

        default:
            return (NuoDB::SqlType)in;
    }
}
