/**
 * (C) Copyright NuoDB, Inc. 2010-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <memory>
#include <vector>

#include "NuoRemote/Bytes.h"
#include "NuoRemote/ResultSet.h"

class GetMapper;
class ResultSetFilter;

/**
 * A class that can be used to map result set values into other
 * values.  User register GetMappers to map results.
 */
class ResultSetMapper : public NuoDB::ResultSet
{
public:
    /**
     * Create a new mapper with the given default mapper and row filter. The
     * ResultSetMapper object now owns the mapper and filter objects and is
     * responsible for deleting it.
     */
    ResultSetMapper(NuoDB::ResultSet* base, GetMapper* newMapper = NULL, ResultSetFilter* newFilter = NULL);

    virtual ~ResultSetMapper(void);

    /**
     * add a GetMapper to this ResultSetMapper. This ResultSetMapper
     * now owns newFilter and is responsible for deleting it.
     * Any previous filter for that column will be deleted
     * @param newMapper a new filter which this ResultSetMapper owns
     */
    void registerGetMapper(GetMapper* newMapper);

    virtual void addRef();
    virtual int  release();
    virtual void close();
    virtual bool next();
    virtual bool wasNull();
    virtual int  findColumn(const char* columName);

    virtual NuoDB::ResultSetMetaData* getMetaData();

    virtual const char*       getString(int columnIndex, int* n_chars);
    virtual const char*       getString(int id);
    virtual const char*       getString(const char* columnName);
    virtual char              getByte(int id);
    virtual char              getByte(const char* columnName);
    virtual bool              getBoolean(int id);
    virtual bool              getBoolean(const char* columnName);
    virtual short             getShort(int id);
    virtual short             getShort(const char* columnName);
    virtual int32_t           getInt(int id);
    virtual int32_t           getInt(const char* columnName);
    virtual int64_t           getLong(int id);
    virtual int64_t           getLong(const char* columnName);
    virtual float             getFloat(int id);
    virtual float             getFloat(const char* columnName);
    virtual double            getDouble(int id);
    virtual double            getDouble(const char* columnName);
    virtual NuoDB::Date*      getDate(int id);
    virtual NuoDB::Date*      getDate(const char* columnName);
    virtual NuoDB::Time*      getTime(int id);
    virtual NuoDB::Time*      getTime(const char* columnName);
    virtual NuoDB::Timestamp* getTimestamp(int id);
    virtual NuoDB::Timestamp* getTimestamp(const char* columnName);
    virtual NuoDB::Blob*      getBlob(int index);
    virtual NuoDB::Blob*      getBlob(const char* columnName);
    virtual NuoDB::Clob*      getClob(int index);
    virtual NuoDB::Clob*      getClob(const char* columnName);
    virtual NuoDB::Bytes      getBytes(int index);
    virtual NuoDB::Bytes      getBytes(const char* columnName);

private:
    NuoDB::ResultSet* base;
    std::vector<std::unique_ptr<GetMapper>> getMappers;
    std::unique_ptr<ResultSetFilter> rowFilter;
    int numColumns;
    int useCount;
};
