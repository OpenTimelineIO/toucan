// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MemoryMap.h"

namespace toucan
{
    MemoryReference::MemoryReference()
    {}

    MemoryReference::MemoryReference(const void* data, size_t size) :
        _data(data),
        _size(size)
    {}

    const void* MemoryReference::getData() const
    {
        return _data;
    }

    size_t MemoryReference::getSize() const
    {
        return _size;
    }

    bool MemoryReference::isValid() const
    {
        return _data != nullptr;
    }

    std::unique_ptr<OIIO::Filesystem::IOMemReader> getMemoryReader(const MemoryReference& ref)
    {
        return ref.isValid() ?
            std::make_unique<OIIO::Filesystem::IOMemReader>(ref.getData(), ref.getSize()) :
            nullptr;
    }
}
