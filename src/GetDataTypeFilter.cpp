/**
 * (C) Copyright 2013-2026 Dassault Systemes SE.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "GetDataTypeFilter.h"

#include "NuoRemote/ResultSet.h"

bool GetDataTypeFilter::accept(NuoDB::ResultSet* rset)
{
    if (colIndex == -1) {
        colIndex = rset->findColumn("DATA_TYPE");
    }
    return rset->getInt(colIndex) == sqlDataType;
}
