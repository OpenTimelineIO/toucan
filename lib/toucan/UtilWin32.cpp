// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Util.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#include <combaseapi.h>

namespace toucan
{
    std::filesystem::path makeUniqueTemp()
    {
        std::string unique;
        GUID guid;
        CoCreateGuid(&guid);
        const uint8_t* guidP = reinterpret_cast<const uint8_t*>(&guid);
        for (int i = 0; i < 16; ++i)
        {
            char buf[3] = "";
            sprintf_s(buf, 3, "%02x", guidP[i]);
            unique += buf;
        }
        std::filesystem::path path = std::filesystem::temp_directory_path() / unique;
        std::filesystem::create_directory(path);
        return path;
    }

    std::string getLastError()
    {
        std::string out;
        const DWORD dw = GetLastError();
        if (dw != ERROR_SUCCESS)
        {
            const size_t bufSize = 4096;
            TCHAR buf[bufSize];
            FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buf,
                bufSize,
                NULL);
            out = std::string(buf, lstrlen(buf));
        }
        return out;
    }
}
