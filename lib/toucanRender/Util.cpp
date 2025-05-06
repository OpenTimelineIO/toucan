// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Util.h"

#include <cctype>
#include <map>
#include <sstream>

namespace toucan
{
    std::string toLower(const std::string& value)
    {
        std::string out;
        for (auto i : value)
        {
            out.push_back(std::tolower(i));
        }
        return out;
    }

    std::pair<std::string, std::string> splitURLProtocol(const std::string& url)
    {
        std::pair<std::string, std::string> out;
        const size_t size = url.size();
        size_t pos = url.find("://");
        if (pos != std::string::npos)
        {
            out.first = url.substr(0, pos + 2);
            out.second = url.substr(pos + 3);
        }
        else
        {
            out.second = url;
        }
        return out;
    }

    std::string getSequenceFrame(
        const std::filesystem::path& path,
        const std::string& namePrefix,
        int frame,
        int padding,
        const std::string& nameSuffix)
    {
        std::stringstream ss;
        ss << namePrefix <<
            std::setw(padding) << std::setfill('0') << frame <<
            nameSuffix;
        return (path / ss.str()).string();
    }

    namespace
    {
        inline bool isNumber(char c)
        {
            return c >= '0' && c <= '9';
        }
    }

    std::pair<std::string, std::string> splitFileNameNumber(const std::string& stem)
    {
        std::pair<std::string, std::string> out;
        size_t i = stem.size();
        if (i > 0)
        {
            for (; i > 0 && isNumber(stem[i - 1]); --i)
                ;
        }
        if (i >= 0)
        {
            out.first = stem.substr(0, i);
            out.second = stem.substr(i, stem.size() - i);
        }
        return out;
    }

    size_t getNumberPadding(const std::string& value)
    {
        size_t out = 0;
        if (!value.empty() && '0' == value[0])
        {
            out = value.size();
        }
        return out;
    }

    std::vector<std::filesystem::path> getSequence(const std::filesystem::path& path)
    {
        std::vector<std::filesystem::path> out;
        const auto split = splitFileNameNumber(path.stem().string());
        if (split.second.empty())
        {
            out.push_back(path);
        }
        else
        {
            std::map<int64_t, std::filesystem::path> map;
            const size_t padding = getNumberPadding(split.second);
            const std::filesystem::path extension = path.extension();
            for (auto const& entry : std::filesystem::directory_iterator(path.parent_path()))
            {
                if (entry.is_regular_file())
                {
                    const auto split2 = splitFileNameNumber(entry.path().stem().string());
                    if (split.first == split2.first &&
                        !split2.second.empty() &&
                        padding == getNumberPadding(split2.second) &&
                        entry.path().extension() == extension)
                    {
                        map[std::atoi(split2.second.c_str())] = entry.path();
                    }
                }
            }
            for (const auto& i : map)
            {
                out.push_back(i.second);
            }
        }
        return out;
    }

    IMATH_NAMESPACE::Box2i fit(
        const IMATH_NAMESPACE::V2i& a,
        const IMATH_NAMESPACE::V2i& b)
    {
        IMATH_NAMESPACE::Box2i out;
        int w = b.x;
        int h = b.y;
        const double aAspect = a.y > 0 ? (a.x / static_cast<double>(a.y)) : 1.0;
        const double bAspect = b.y > 0 ? (b.x / static_cast<double>(b.y)) : 1.0;
        if (bAspect > aAspect)
        {
            w = a.x;
            h = w / bAspect;
        }
        else
        {
            h = a.y;
            w = h * bAspect;
        }
        out.min.x = a.x / 2 - w / 2;
        out.min.y = a.y / 2 - h / 2;
        out.max.x = out.min.x + w - 1;
        out.max.y = out.min.y + h - 1;
        return out;
    }

    OTIO_NS::AnyVector vecToAny(const IMATH_NAMESPACE::V2i& vec)
    {
        return OTIO_NS::AnyVector
        {
            static_cast<int64_t>(vec.x),
            static_cast<int64_t>(vec.y)
        };
    }
    
    OTIO_NS::AnyVector vecToAny(const IMATH_NAMESPACE::V4f& vec)
    {
        return OTIO_NS::AnyVector
        {
            static_cast<double>(vec.x),
            static_cast<double>(vec.y),
            static_cast<double>(vec.z),
            static_cast<double>(vec.w)
        };
    }
    
    void anyToVec(const OTIO_NS::AnyVector& any, IMATH_NAMESPACE::V2i& out)
    {
        if (2 == any.size())
        {
            out.x = std::any_cast<int64_t>(any[0]);
            out.y = std::any_cast<int64_t>(any[1]);
        }
    }
    
    void anyToVec(const OTIO_NS::AnyVector& any, IMATH_NAMESPACE::V4f& out)
    {
        if (4 == any.size())
        {
            out.x = std::any_cast<double>(any[0]);
            out.y = std::any_cast<double>(any[1]);
            out.z = std::any_cast<double>(any[2]);
            out.w = std::any_cast<double>(any[3]);
        }
    }

    std::vector<std::filesystem::path> getOpenFXPluginPaths(
        const std::filesystem::path& executablePath)
    {
        const std::filesystem::path parentPath = executablePath.parent_path();
        std::vector<std::filesystem::path> searchPath;

        // Add executable directory and relative paths
        searchPath.push_back(parentPath);
#if defined(_WINDOWS)
        searchPath.push_back(parentPath / ".." / ".." / "..");
        // Add standard Windows OpenFX plugin path
        searchPath.push_back("C:\\Program Files\\Common Files\\OFX\\Plugins");
#else // _WINDOWS
        searchPath.push_back(parentPath / ".." / "..");
#if defined(__APPLE__)
        // Add standard macOS OpenFX plugin path
        searchPath.push_back("/Library/OFX/Plugins");
#else
        // Add standard Linux/Unix OpenFX plugin path
        searchPath.push_back("/usr/OFX/Plugins");
#endif // __APPLE__
#endif // _WINDOWS

        // Add paths from environment variable OFX_PLUGIN_PATH if it exists
        if (const char* envPath = std::getenv("OFX_PLUGIN_PATH")) {
#if defined(_WINDOWS)
            const char delimiter = ';';
#else
            const char delimiter = ':';
#endif
            std::string envPathStr(envPath);
            size_t pos = 0;
            std::string token;
            while ((pos = envPathStr.find(delimiter)) != std::string::npos) {
                token = envPathStr.substr(0, pos);
                if (!token.empty()) {
                    searchPath.push_back(token);
                }
                envPathStr.erase(0, pos + 1);
            }
            if (!envPathStr.empty()) {
                searchPath.push_back(envPathStr);
            }
        }

        return searchPath;
    }
}
