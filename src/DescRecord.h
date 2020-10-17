/**
 * (C) Copyright NuoDB, Inc. 2011-2020  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include <string>

#include "OdbcBase.h"

class DescRecord
{
public:
    DescRecord() = default;
    ~DescRecord() = default;

    std::string name;
    void*       dataPtr = nullptr;
    SQLINTEGER* octetLengthPtr = nullptr;
    SQLINTEGER* indicatorPtr = nullptr;
    int         type = 0;
    int         subType = 0;
    int         length = 0;
    int         precision = 0;
    int         scale = 0;
    int         bufferLength = 0;
    bool        nullable = false;
};
