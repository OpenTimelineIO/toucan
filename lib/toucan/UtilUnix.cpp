// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Util.h"

#include <stdlib.h>

namespace toucan
{
    std::filesystem::path makeUniqueTemp()
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() / "XXXXXX";
        return mkdtemp(path.string().c_str());
    }
}
