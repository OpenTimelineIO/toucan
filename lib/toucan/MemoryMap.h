// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <OpenImageIO/filesystem.h>

#include <filesystem>
#include <map>
#include <string>

namespace toucan
{
    //! A memory-mapped file for reading .otioz files without
    //! extracting the contents.
    class MemoryMap
    {
    public:
        MemoryMap(const std::filesystem::path&);

        ~MemoryMap();

        const void* getData() const;

        const size_t getSize() const;

    private:
        struct Private;
        std::unique_ptr<Private> _p;
    };

    //! A reference to media within a memory-mapped .otioz file.
    class MemoryReference
    {
    public:
        MemoryReference();
        MemoryReference(const void* data, size_t size);

        const void* getData() const;

        size_t getSize() const;

        bool isValid() const;

    private:
        const void* _data = nullptr;
        size_t _size = 0;
    };

    //! Map URLs to memory references.
    typedef std::map<std::string, MemoryReference> MemoryReferences;

    //! Get an OIIO memory reader for a memory reference.
    std::unique_ptr<OIIO::Filesystem::IOMemReader> getMemoryReader(const MemoryReference&);
}
