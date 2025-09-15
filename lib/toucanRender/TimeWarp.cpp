// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimeWarp.h"

namespace toucan
{
    /*LinearTimeWarpNode::LinearTimeWarpNode(
        double timeScalar,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode("LinearTimeWarp", inputs),
        _timeScalar(timeScalar)
    {}

    LinearTimeWarpNode::~LinearTimeWarpNode()
    {}

    OIIO::ImageBuf LinearTimeWarpNode::exec(const OTIO_NS::RationalTime& t)
    {
        OIIO::ImageBuf buf;
        if (!_inputs.empty())
        {
            const OTIO_NS::RationalTime scaledTime = OTIO_NS::RationalTime(t.value() * _timeScalar, t.rate()).floor();
            buf = _inputs[0]->exec(scaledTime);
        }
        return buf;
    }*/
}
