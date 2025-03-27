// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MemoryMap.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

namespace toucan
{
    struct MemoryMap::Private
    {
        int f = -1;
        size_t size = 0;
        void* mmap = reinterpret_cast<void*>(-1);
    };

    MemoryMap::MemoryMap(const std::filesystem::path& path) :
        _p(new Private)
    {
        int openFlags = O_RDONLY;
        int openMode  = 0;
        _p->f = open(path.u8string().c_str(), openFlags, openMode);
        if (-1 == _p->f)
        {
            throw std::runtime_error("Cannot open file: " + path.string());
        }

        _p->size = std::filesystem::file_size(path);

        _p->mmap = mmap(0, _p->size, PROT_READ, MAP_SHARED, _p->f, 0);
        madvise(_p->mmap, _p->size, MADV_SEQUENTIAL | MADV_SEQUENTIAL);
        if (_p->mmap == (void*)-1)
        {
            throw std::runtime_error("Cannot memory-map file: " + path.string());
        }
    }

    MemoryMap::~MemoryMap()
    {
        if (_p->mmap != (void*)-1)
        {
            munmap(_p->mmap, _p->size);
        }
        if (_p->f != -1)
        {
            close(_p->f);
        }
    }

    const void* MemoryMap::getData() const
    {
        return reinterpret_cast<const uint8_t*>(_p->mmap);
    }

    const size_t MemoryMap::getSize() const
    {
        return _p->size;
    }
}
