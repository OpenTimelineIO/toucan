// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MemoryMap.h"

namespace toucan
{
    struct MemoryMap::Private
    {
        void* data = nullptr;
        size_t size = 0;
    };

    MemoryMap::MemoryMap(const std::filesystem::path& path) :
        _p(new Private)
    {

    }

    MemoryMap::~MemoryMap()
    {}

    const void* MemoryMap::getData() const
    {
        return _p->data;
    }

    const size_t MemoryMap::getSize() const
    {
        return _p->size;
    }
}
