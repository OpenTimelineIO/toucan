// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "CompTest.h"

#include <toucanRender/Read.h>
#include <toucanRender/Comp.h>

namespace toucan
{
    void compTest(const std::filesystem::path& path)
    {
        std::cout << "compTest" << std::endl;
        auto fg = std::make_shared<ImageReadNode>(path / "Letter_A.png");
        auto bg = std::make_shared<ImageReadNode>(path / "Gradient.png");
        auto comp = std::make_shared<CompNode>(
            std::vector<std::shared_ptr<IImageNode> >{ fg, bg });
        comp->setPremult(true);
        auto buf = comp->exec(OTIO_NS::RationalTime(0.0, 24.0));
        buf.write("compTest.png");
    }
}
