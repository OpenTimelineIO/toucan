// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>

#include <feather-tk/core/Context.h>

namespace toucan
{
    void filesModelTest(
        const std::shared_ptr<feather_tk::Context>&,
        const std::shared_ptr<ImageEffectHost>&,
        const std::filesystem::path& path);
}
