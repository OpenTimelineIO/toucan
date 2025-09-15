// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ReadTest.h"

#include <toucanRender/Read.h>

#include <cassert>

namespace toucan
{
    void readTest(const std::filesystem::path& path)
    {
        std::cout << "readTest" << std::endl;
        auto read = std::make_shared<ImageReadNode>(path / "Letter_A.png");
        read->setTime(OTIO_NS::RationalTime(0.0, 24.0));
        auto buf = read->exec();
        const auto& spec = buf.spec();
        assert(spec.width > 0);
    }
}
