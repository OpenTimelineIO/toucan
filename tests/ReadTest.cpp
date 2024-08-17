// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "ReadTest.h"

#include <toucan/Read.h>

#include <cassert>

namespace toucan
{
    void readTest(
        const std::filesystem::path& path,
        const std::shared_ptr<ImageEffectHost>& host)
    {
        std::cout << "readTest" << std::endl;
        auto read = std::make_shared<ReadNode>(path / "Letter_A.png");
        auto buf = read->exec(OTIO_NS::RationalTime(), host);
        const auto& spec = buf.spec();
        assert(spec.width > 0);
    }
}
