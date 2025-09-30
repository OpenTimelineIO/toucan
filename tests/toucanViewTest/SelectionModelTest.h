// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <ftk/Core/Context.h>

#include <filesystem>

namespace toucan
{
    void selectionModelTest(
        const std::shared_ptr<ftk::Context>&,
        const std::filesystem::path& path);
}
