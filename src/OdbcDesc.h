/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "OdbcBase.h"
#include "OdbcObject.h"

enum OdbcDescType
{
    odtApplicationParameter,
    odtImplementationParameter,
    odtApplicationRow,
    odtImplementationRow
};

class OdbcConnection;
class DescRecord;

class OdbcDesc : public OdbcObject
{
public:
    OdbcDesc(OdbcDescType type, OdbcConnection* connect)
        : connection(connect),
          descType(type)
    {}
    virtual ~OdbcDesc();

    virtual OdbcObjectType getType() { return odbcTypeDescriptor; }

    DescRecord* getDescRecord(int number);
    RETCODE sqlSetDescField(int recNumber, int fieldId, SQLPOINTER value, int length);

    OdbcConnection* connection;
    DescRecord**    records = nullptr;
    int             recordSlots = 0;
    OdbcDescType    descType;
};
