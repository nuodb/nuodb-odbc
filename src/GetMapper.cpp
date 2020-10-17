/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include <cstdint>

#include "GetMapper.h"

#define GEN(NAME, TYPE)                         \
    TYPE GetMapper::get ## NAME(TYPE value)     \
    {                                           \
        return value;                           \
    }

GEN(Blob, NuoDB::Blob*)
GEN(Clob, NuoDB::Clob*)
GEN(Time, NuoDB::Time*)
GEN(Bytes, NuoDB::Bytes)
GEN(Date, NuoDB::Date*)
GEN(Timestamp, NuoDB::Timestamp*)
GEN(BigDecimal, NuoDB::BigDecimal*)

GEN(Boolean, bool)
GEN(Byte, char)
GEN(Short, short)
GEN(Int, int32_t)
GEN(Long, int64_t)
GEN(Float, float)
GEN(Double, double)
GEN(String, const char*)

#undef GEN
