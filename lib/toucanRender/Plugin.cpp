// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Plugin.h"

namespace toucan
{
    int Plugin::getCount()
    {
        return _getNumberOfPlugins ? _getNumberOfPlugins() : 0;
    }

    OfxPlugin* Plugin::getPlugin(int index)
    {
        return _getPlugin ? _getPlugin(index) : nullptr;
    }
    
    namespace
    {
        void _findPlugins(
            const std::filesystem::path& path,
            std::vector<std::filesystem::path>& out,
            int depth,
            int maxDepth)
        {
            try
            {
                for (auto const& entry : std::filesystem::directory_iterator(path))
                {
                    const auto& entryPath = entry.path();
                    if (entry.is_regular_file() && entryPath.extension() == ".ofx")
                    {
                        out.push_back(entryPath);
                    }
                    else if (entry.is_directory() && depth < maxDepth)
                    {
                        _findPlugins(entryPath, out, depth + 1, maxDepth);
                    }
                }
            }
            catch (const std::exception&)
            {
                //! \bug How should this be handled?
            }
        }
    }

    void findPlugins(
        const std::filesystem::path& path,
        std::vector<std::filesystem::path>& out)
    {
        _findPlugins(path, out, 0, 10);
    }
}
