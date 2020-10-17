/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "GetMapper.h"

#include <NuoRemote/ResultSetMetaData.h>

/**
 * Used with a ResultSetMapper to map types returned from the server
 */
class OdbcTypeMapper : public GetMapper
{
public:
   // currently we only need to map DATA_TYPE
    OdbcTypeMapper(const char* columnToMap="DATA_TYPE")
        : GetMapper(columnToMap)
    {}
    virtual ~OdbcTypeMapper() = default;

    virtual short   getShort(short value);
    virtual int32_t getInt(int32_t value);
    virtual int64_t getLong(int64_t value);

    static NuoDB::SqlType mapType(int in);
};
