/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#pragma once

#include "OdbcBase.h"

#include <string>
#include <vector>

#include "OdbcObject.h"

struct Binding
{
    void reset() { dataAtExecLength = 0; offset = 0; count = 0; }

    PTR pointer = nullptr;
    PTR indicatorPointer = nullptr;
    std::string accumulator;
    SQLLEN bufferLength = 0;
    SQLLEN dataAtExecLength = 0;
    SQLLEN offset = 0;
    int type = 0;
    int cType = 0;
    int sqlType = 0;
    int count = 0;
};

class Bindings final
{
public:
    Bindings() = default;
    ~Bindings() = default;

    int  getCount() const    { return (int)bindings.size(); }

    void alloc(int newCount) { bindings.resize(newCount); }
    void release()           { bindings.clear(); }

    void reset()
    {
        for (auto& binding : bindings) {
            binding.reset();
        }
    }

    Binding* getBinding(int index)
    {
        if (index > (int)bindings.size()) {
            return nullptr;
        }
        return &bindings[index-1];
    }

private:
    std::vector<Binding> bindings;
};
