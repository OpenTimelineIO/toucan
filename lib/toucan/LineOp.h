// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#pragma once

#include <toucan/ImageOp.h>

namespace toucan
{
    //! Line data.
    struct LineData
    {
        IMATH_NAMESPACE::V2i pos1  = IMATH_NAMESPACE::V2i(0, 0);
        IMATH_NAMESPACE::V2i pos2  = IMATH_NAMESPACE::V2i(0, 0);
        IMATH_NAMESPACE::V4f color = IMATH_NAMESPACE::V4f(1.F, 1.F, 1.F, 1.F);
        bool skipFirstPoint = false;
    };

    //! Line drawing operation.
    class LineOp : public IImageOp
    {
    public:
        LineOp(
            const LineData& = LineData(),
            const std::vector<std::shared_ptr<IImageOp> >& = {});

        virtual ~LineOp();

        const LineData& getData() const;
        void setData(const LineData&);

        OIIO::ImageBuf exec(const OTIO_NS::RationalTime&) override;

    private:
        LineData _data;
    };

    //! Line drawing OTIO effect.
    class LineEffect : public IEffect
    {
    public:
        struct Schema
        {
            static auto constexpr name = "LineEffect";
            static int constexpr version = 1;
        };

        LineEffect(
            std::string const& name = std::string(),
            std::string const& effect_name = std::string(),
            OTIO_NS::AnyDictionary const& metadata = OTIO_NS::AnyDictionary());

        std::shared_ptr<IImageOp> createOp(
            const std::vector<std::shared_ptr<IImageOp> >& inputs) override;

    protected:
        virtual ~LineEffect();

        bool read_from(Reader&) override;
        void write_to(Writer&) const override;

    private:
        LineData _data;
    };
}
