// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MemoryMap.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <windows.h>

namespace toucan
{
    struct MemoryMap::Private
    {
        HANDLE h = INVALID_HANDLE_VALUE;
        void* mmap = nullptr;
        const void* data = nullptr;
        size_t size = 0;
    };

    MemoryMap::MemoryMap(const std::filesystem::path& path) :
        _p(new Private)
    {
        const std::wstring wpath = path.wstring();
        _p->h = CreateFileW(
            wpath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0);
        if (INVALID_HANDLE_VALUE == _p->h)
        {
            throw std::runtime_error("Cannot open file: " + path.string());
        }

        struct _stati64 s;
        memset(&s, 0, sizeof(struct _stati64));
        if (_wstati64(wpath.c_str(), &s) != 0)
        {
            throw std::runtime_error("Cannot stat file: " + path.string());
        }
        _p->size = s.st_size;

        _p->mmap = CreateFileMapping(_p->h, 0, PAGE_READONLY, 0, 0, 0);
        if (!_p->mmap)
        {
            throw std::runtime_error("Cannot memory-map file: " + path.string());
        }

        _p->data = reinterpret_cast<const void*> (
            MapViewOfFile(_p->mmap, FILE_MAP_READ, 0, 0, 0));
        if (!_p->mmap)
        {
            throw std::runtime_error("Cannot map view of file: " + path.string());
        }
    }

    MemoryMap::~MemoryMap()
    {
        if (_p->data)
        {
            UnmapViewOfFile(_p->data);
        }
        if (_p->mmap)
        {
            CloseHandle(_p->mmap);
        }
        if (_p->h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_p->h);
        }
    }

    const void* MemoryMap::getData() const
    {
        return _p->data;
    }

    const size_t MemoryMap::getSize() const
    {
        return _p->size;
    }
}
