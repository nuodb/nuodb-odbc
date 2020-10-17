/**
 * (C) Copyright NuoDB, Inc. 2013-2020  All Rights Reserved
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

namespace NuoDB {
class ResultSet;
}

/**
 * A Class that is used to dynamically filter an existing ResultSet
 */
class ResultSetFilter
{
public:
    virtual ~ResultSetFilter(void) = default;

    virtual bool accept(NuoDB::ResultSet* rset) = 0;
};
