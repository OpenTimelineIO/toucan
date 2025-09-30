// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <ftk/Ui/IWidget.h>

namespace toucan
{
    //! Timeline item label.
    class ItemLabel : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ItemLabel();

        //! Create a new label.
        static std::shared_ptr<ItemLabel> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the name.
        void setName(const std::string&);

        //! Set the duration.
        void setDuration(const std::string&);

        //! Set the margin size role.
        void setMarginRole(ftk::SizeRole);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void clipEvent(const ftk::Box2I&, bool) override;
        void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

    private:
        std::string _name;
        std::string _duration;
        ftk::SizeRole _marginRole = ftk::SizeRole::MarginInside;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int margin = 0;
            int margin2 = 0;
            ftk::FontInfo fontInfo;
            ftk::FontMetrics fontMetrics;
            ftk::Size2I nameSize;
            ftk::Size2I durationSize;
        };
        SizeData _size;

        struct DrawData
        {
            std::vector<std::shared_ptr<ftk::Glyph> > nameGlyphs;
            std::vector<std::shared_ptr<ftk::Glyph> > durationGlyphs;
        };
        DrawData _draw;
    };
}

