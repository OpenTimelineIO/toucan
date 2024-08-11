// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "CheckersOp.h"

#include <OpenImageIO/imagebufalgo.h>

namespace toucan
{
    CheckersOp::CheckersOp(
        const CheckersData& data,
        const OTIO_NS::RationalTime& timeOffset,
        const std::vector<std::shared_ptr<IImageOp> >& inputs) :
        IImageOp(timeOffset, inputs),
        _data(data)
    {}

    CheckersOp::~CheckersOp()
    {}

    const CheckersData& CheckersOp::getData() const
    {
        return _data;
    }

    void CheckersOp::setData(const CheckersData& value)
    {
        _data = value;
    }

    OIIO::ImageBuf CheckersOp::exec(const OTIO_NS::RationalTime& time)
    {
        OIIO::ImageBuf buf;
        if (!_inputs.empty())
        {
            buf = _inputs[0]->exec(time);
            OIIO::ImageBufAlgo::checker(
                buf,
                _data.checkerSize.x,
                _data.checkerSize.y,
                1,
                { _data.color1.x, _data.color1.y, _data.color1.z, _data.color1.w },
                { _data.color2.x, _data.color2.y, _data.color2.z, _data.color2.w });
        }
        else
        {
            buf = OIIO::ImageBufAlgo::checker(
                _data.checkerSize.x,
                _data.checkerSize.y,
                1,
                { _data.color1.x, _data.color1.y, _data.color1.z, _data.color1.w },
                { _data.color2.x, _data.color2.y, _data.color2.z, _data.color2.w },
                0,
                0,
                0,
                OIIO::ROI(0, _data.size.x, 0, _data.size.y, 0, 1, 0, 4));
        }
        return buf;
    }
    
    CheckersEffect::CheckersEffect(
        std::string const& name,
        std::string const& effect_name,
        OTIO_NS::AnyDictionary const& metadata) :
        IEffect(name, effect_name, metadata)
    {}

    CheckersEffect::~CheckersEffect()
    {}

    const CheckersData& CheckersEffect::getData() const
    {
        return _data;
    }

    void CheckersEffect::setData(const CheckersData & value)
    {
        _data = value;
    }

    std::shared_ptr<IImageOp> CheckersEffect::createOp(
        const OTIO_NS::RationalTime& timeOffset,
        const std::vector<std::shared_ptr<IImageOp> >& inputs)
    {
        return std::make_shared<CheckersOp>(_data, timeOffset, inputs);
    }

    bool CheckersEffect::read_from(Reader& reader)
    {
        int64_t width = 0;
        int64_t height = 0;
        int64_t checkerWidth = 0;
        int64_t checkerHeight = 0;
        IMATH_NAMESPACE::V4d color1;
        IMATH_NAMESPACE::V4d color2;
        bool out =
            reader.read("width", &width) &&
            reader.read("height", &height) &&
            reader.read("checkerWidth", &width) &&
            reader.read("checkerHeight", &height) &&
            reader.read("red1", &color1.x) &&
            reader.read("green1", &color1.y) &&
            reader.read("blue1", &color1.z) &&
            reader.read("alpha1", &color1.w) &&
            reader.read("red2", &color2.x) &&
            reader.read("green2", &color2.y) &&
            reader.read("blue2", &color2.z) &&
            reader.read("alpha2", &color2.w) &&
            IEffect::read_from(reader);
        if (out)
        {
            _data.size.x = width;
            _data.size.y = height;
            _data.color1 = color1;
            _data.color2 = color2;
        }
        return out;
    }

    void CheckersEffect::write_to(Writer& writer) const
    {
        IEffect::write_to(writer);
        writer.write("width", static_cast<int64_t>(_data.size.x));
        writer.write("height", static_cast<int64_t>(_data.size.y));
        writer.write("width", static_cast<int64_t>(_data.checkerSize.x));
        writer.write("height", static_cast<int64_t>(_data.checkerSize.y));
        writer.write("red1", static_cast<double>(_data.color1.x));
        writer.write("green1", static_cast<double>(_data.color1.y));
        writer.write("blue1", static_cast<double>(_data.color1.z));
        writer.write("alpha1", static_cast<double>(_data.color1.w));
        writer.write("red2", static_cast<double>(_data.color2.x));
        writer.write("green2", static_cast<double>(_data.color2.y));
        writer.write("blue2", static_cast<double>(_data.color2.z));
        writer.write("alpha2", static_cast<double>(_data.color2.w));
    }
}
