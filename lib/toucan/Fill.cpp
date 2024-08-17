// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "Fill.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

namespace toucan
{
    FillNode::FillNode(
        const FillData& data,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode(inputs),
        _data(data)
    {}

    FillNode::~FillNode()
    {}

    const FillData& FillNode::getData() const
    {
        return _data;
    }

    void FillNode::setData(const FillData& value)
    {
        _data = value;
    }

    OIIO::ImageBuf FillNode::exec(
        const OTIO_NS::RationalTime& time,
        const std::shared_ptr<ImageEffectHost>& host)
    {
        OIIO::ImageBuf buf;
        if (!_inputs.empty() && _inputs[0])
        {
            OTIO_NS::RationalTime offsetTime = time;
            if (!_timeOffset.is_invalid_time())
            {
                offsetTime -= _timeOffset;
            }
            buf = _inputs[0]->exec(offsetTime, host);
            OIIO::ImageBufAlgo::fill(
                buf,
                { _data.color.x, _data.color.y, _data.color.z, _data.color.w });
        }
        else
        {
            buf = OIIO::ImageBufAlgo::fill(
                { _data.color.x, _data.color.y, _data.color.z, _data.color.w },
                OIIO::ROI(0, _data.size.x, 0, _data.size.y, 0, 1, 0, 4));
        }
        return buf;
    }
    
    FillEffect::FillEffect(
        std::string const& name,
        std::string const& effect_name,
        OTIO_NS::AnyDictionary const& metadata) :
        IEffect(name, effect_name, metadata)
    {}

    FillEffect::~FillEffect()
    {}

    std::shared_ptr<IImageNode> FillEffect::createNode(
        const std::vector<std::shared_ptr<IImageNode> >& inputs)
    {
        return std::make_shared<FillNode>(_data, inputs);
    }

    bool FillEffect::read_from(Reader& reader)
    {
        OTIO_NS::AnyVector size;
        OTIO_NS::AnyVector color;
        bool out =
            reader.read("size", &size) &&
            reader.read("color", &color) &&
            IEffect::read_from(reader);
        if (out)
        {
            anyToVec(size, _data.size);
            anyToVec(color, _data.color);
        }
        return out;
    }

    void FillEffect::write_to(Writer& writer) const
    {
        IEffect::write_to(writer);
        writer.write("size", vecToAny(_data.size));
        writer.write("color", vecToAny(_data.color));
    }
}
