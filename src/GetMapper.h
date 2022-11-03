/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "OdbcBase.h"

#include <cstdint>
#include <string>

#include "NuoRemote/Bytes.h"

namespace NuoDB {
class BigDecimal;
class Blob;
class Clob;
class Date;
class RowId;
class SQLWarning;
class Time;
class Timestamp;
class TimestampNoTZ;
}

/**
 * A Class that is used to provide mappings for get* methods in ResultSetMapper.
 * This class provides a default implementation of all the methods which just
 * call through to the ResultSetMapper.  To use: create a new child class
 * which overrides appropriate methods
 */
class GetMapper
{
public:
    GetMapper(const char* columnToMap)
        : columnToMap(columnToMap)
    {}

    virtual ~GetMapper() = default;

    const char* getColumn() const { return columnToMap.c_str(); }

    virtual const char* getString(const char* value);
    virtual char        getByte(char value);
    virtual bool        getBoolean(bool value);
    virtual short       getShort(short value);
    virtual int32_t     getInt(int32_t value);
    virtual int64_t     getLong(int64_t value);
    virtual float       getFloat(float value);
    virtual double      getDouble(double value);

    virtual NuoDB::Blob*    getBlob(NuoDB::Blob* value);
    virtual NuoDB::Clob*    getClob(NuoDB::Clob* value);

    virtual NuoDB::Bytes          getBytes(NuoDB::Bytes value);
    virtual NuoDB::Date*          getDate(NuoDB::Date* value);
    virtual NuoDB::Time*          getTime(NuoDB::Time* value);
    virtual NuoDB::Timestamp*     getTimestamp(NuoDB::Timestamp* value);
    virtual NuoDB::TimestampNoTZ* getTimestampNoTZ(NuoDB::TimestampNoTZ* value);
    virtual NuoDB::BigDecimal*    getBigDecimal(NuoDB::BigDecimal* value);

private:
    std::string columnToMap;
};
