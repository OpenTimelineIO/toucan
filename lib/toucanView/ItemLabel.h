// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/IWidget.h>

namespace toucan
{
    //! Timeline item label.
    class ItemLabel : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ItemLabel();

        //! Create a new label.
        static std::shared_ptr<ItemLabel> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the name.
        void setName(const std::string&);

        //! Set the duration.
        void setDuration(const std::string&);

        //! Set the margin size role.
        void setMarginRole(feather_tk::SizeRole);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void clipEvent(const feather_tk::Box2I&, bool) override;
        void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

    private:
        std::string _name;
        std::string _duration;
        feather_tk::SizeRole _marginRole = feather_tk::SizeRole::MarginInside;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int margin = 0;
            int margin2 = 0;
            feather_tk::FontInfo fontInfo;
            feather_tk::FontMetrics fontMetrics;
            feather_tk::Size2I nameSize;
            feather_tk::Size2I durationSize;
        };
        SizeData _size;

        struct DrawData
        {
            std::vector<std::shared_ptr<feather_tk::Glyph> > nameGlyphs;
            std::vector<std::shared_ptr<feather_tk::Glyph> > durationGlyphs;
        };
        DrawData _draw;
    };
}

