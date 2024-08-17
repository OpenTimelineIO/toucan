// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "Pow.h"

#include <OpenImageIO/imagebufalgo.h>

namespace toucan
{
    PowNode::PowNode(
        const PowData& data,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode(inputs),
        _data(data)
    {}

    PowNode::~PowNode()
    {}

    const PowData& PowNode::getData() const
    {
        return _data;
    }

    void PowNode::setData(const PowData& value)
    {
        _data = value;
    }

    OIIO::ImageBuf PowNode::exec(
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
            buf = OIIO::ImageBufAlgo::pow(
                _inputs[0]->exec(offsetTime, host),
                _data.value);
        }
        return buf;
    }
    
    PowEffect::PowEffect(
        std::string const& name,
        std::string const& effect_name,
        OTIO_NS::AnyDictionary const& metadata) :
        IEffect(name, effect_name, metadata)
    {}

    PowEffect::~PowEffect()
    {}

    std::shared_ptr<IImageNode> PowEffect::createNode(
        const std::vector<std::shared_ptr<IImageNode> >& inputs)
    {
        return std::make_shared<PowNode>(_data, inputs);
    }

    bool PowEffect::read_from(Reader& reader)
    {
        double value = 0.0;
        bool out =
            reader.read("value", &value) &&
            IEffect::read_from(reader);
        if (out)
        {
            _data.value = value;
        }
        return out;
    }

    void PowEffect::write_to(Writer& writer) const
    {
        IEffect::write_to(writer);
        writer.write("value", static_cast<double>(_data.value));
    }
}
