// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Comp.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

namespace toucan
{
    CompNode::CompNode(const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode("Comp", inputs)
    {}

    CompNode::~CompNode()
    {}

    void CompNode::setPremult(bool premult)
    {
        _premult = premult;
    }

    void CompNode::setResize(bool resize)
    {
        _resize = resize;
    }

    OIIO::ImageBuf CompNode::exec(const OTIO_NS::RationalTime& t)
    {
        OIIO::ImageBuf buf;
        if (_inputs.size() > 1 && _inputs[0] && _inputs[1])
        {
            auto fgBuf = _inputs[0]->exec(t);
            buf = _inputs[1]->exec(t);
            const auto fgSpec = fgBuf.spec();
            if (_premult &&
                fgSpec.width > 0 &&
                fgSpec.height > 0)
            {
                fgBuf = OIIO::ImageBufAlgo::premult(fgBuf);
            }
            const auto& bgSpec = buf.spec();
            if (fgSpec.width > 0 && fgSpec.height > 0 &&
                bgSpec.width > 0 && bgSpec.height > 0 &&
                (fgSpec.width != bgSpec.width || fgSpec.height != bgSpec.height))
            {
                IMATH_NAMESPACE::Box2i fit = toucan::fit(
                    IMATH_NAMESPACE::V2i(bgSpec.width, bgSpec.height),
                    IMATH_NAMESPACE::V2i(fgSpec.width, fgSpec.height));
                const auto resizedBuf = OIIO::ImageBufAlgo::resize(
                    fgBuf,
                    "",
                    0.0,
                    OIIO::ROI(0, fit.max.x - fit.min.x + 1, 0, fit.max.y - fit.min.y + 1));
                fgBuf = OIIO::ImageBuf(OIIO::ImageSpec(
                    bgSpec.width,
                    bgSpec.height,
                    bgSpec.nchannels,
                    bgSpec.format));
                OIIO::ImageBufAlgo::paste(
                    fgBuf,
                    fit.min.x,
                    fit.min.y,
                    0,
                    0,
                    resizedBuf);
            }
            if (fgSpec.width > 0 &&
                fgSpec.height > 0)
            {
                buf = OIIO::ImageBufAlgo::over(fgBuf, buf);
            }
        }
        else if (1 == _inputs.size() && _inputs[0])
        {
            buf = _inputs[0]->exec(t);
            if (_premult)
            {
                buf = OIIO::ImageBufAlgo::premult(buf);
            }
        }
        return buf;
    }
}
