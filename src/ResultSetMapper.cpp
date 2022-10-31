/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include <string.h>
#include <cassert>

#include "ResultSetMapper.h"

#include "NuoRemote/ResultSetMetaData.h"
#include "GetMapper.h"
#include "ResultSetFilter.h"

using namespace NuoDB;

ResultSetMapper::ResultSetMapper(ResultSet* b, GetMapper* mapper, ResultSetFilter* newFilter)
    : base(b),
      useCount(1)
{
    base->addRef();
    numColumns = base->getMetaData()->getColumnCount();
    getMappers.resize(numColumns);
    if (mapper) {
        registerGetMapper(mapper);
    }
    rowFilter.reset(newFilter);
}

ResultSetMapper::~ResultSetMapper(void)
{
    base->release();
}

void ResultSetMapper::registerGetMapper(GetMapper* mapper)
{
    // findColumn returns 1 based column #
    int column = findColumn(mapper->getColumn()) - 1;
    getMappers[column].reset(mapper);
}

bool ResultSetMapper::wasNull()
{
    return base->wasNull();
}

int ResultSetMapper::findColumn(const char* columnName)
{
    return base->findColumn(columnName);
}

void ResultSetMapper::close()
{
    return base->close();
}

bool ResultSetMapper::next()
{
    bool hasNext = base->next();
    while (hasNext && rowFilter && !rowFilter->accept(base)) {
        hasNext = base->next();
    }
    return hasNext;
}

ResultSetMetaData* ResultSetMapper::getMetaData()
{
    return base->getMetaData();
}

int ResultSetMapper::release()
{
    if (--useCount == 0) {
        delete this;

        return 0;
    }

    return useCount;
}

void ResultSetMapper::addRef()
{
    ++useCount;
}

const char* ResultSetMapper::getString(int columnIndex, int* n_chars)
{
    return base->getString(columnIndex, n_chars);
}

#define GEN(NAME, TYPE)                                                 \
    TYPE ResultSetMapper::get ## NAME(const char* column)               \
    {                                                                   \
        return get ## NAME(base->findColumn(column));                   \
    }                                                                   \
    TYPE ResultSetMapper::get ## NAME(int column)                       \
    {                                                                   \
        assert(base);                                                   \
        GetMapper* gf = getMappers[column-1].get();                     \
                                                                        \
        if (gf) {                                                       \
            return gf->get ## NAME(base->get ## NAME(column));          \
        }                                                               \
                                                                        \
        return base->get ## NAME(column);                               \
    }

GEN(String, const char*)
GEN(Byte, char)
GEN(Boolean, bool)
GEN(Short, short)
GEN(Int, int32_t)
GEN(Long, int64_t)
GEN(Float, float)
GEN(Double, double)

GEN(Blob, NuoDB::Blob*)
GEN(Clob, NuoDB::Clob*)
GEN(Bytes, NuoDB::Bytes)
GEN(Time, NuoDB::Time*)
GEN(Date, NuoDB::Date*)
GEN(Timestamp, NuoDB::Timestamp*)
GEN(TimestampNoTZ, NuoDB::TimestampNoTZ*)

#undef GEN
