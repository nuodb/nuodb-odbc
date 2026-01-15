/**
 * (C) Copyright 2013-2026 Dassault Systemes SE.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "ResultSetFilter.h"

class GetDataTypeFilter : public ResultSetFilter
{
public:
    GetDataTypeFilter(int singleDataType)
        : sqlDataType(singleDataType)
    {}

    virtual bool accept(NuoDB::ResultSet* rset);

private:
    int colIndex = -1;
    int sqlDataType;
};
