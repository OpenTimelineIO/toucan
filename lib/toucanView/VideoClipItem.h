// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IItem.h>
#include <toucanView/ItemLabel.h>
#include <toucanView/MarkerItem.h>
#include <toucanView/ThumbnailsWidget.h>

#include <ftk/Ui/RowLayout.h>

#include <opentimelineio/clip.h>

namespace toucan
{
    //! Timeline video clip item.
    class VideoClipItem : public IItem
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const ItemData&,
            const OTIO_NS::Clip*,
            const ftk::Color4F&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~VideoClipItem();

        //! Create a new item.
        static std::shared_ptr<VideoClipItem> create(
            const std::shared_ptr<ftk::Context>&,
            const ItemData&,
            const OTIO_NS::Clip*,
            const ftk::Color4F&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setScale(double) override;

        void setGeometry(const ftk::Box2I&) override;
        ftk::Box2I getChildrenClipRect() const override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

    protected:
        void _timeUnitsUpdate() override;
        void _buildMenu(const std::shared_ptr<ftk::Menu>&) override;

    private:
        void _textUpdate();

        const OTIO_NS::Clip* _clip = nullptr;
        std::string _text;
        ftk::Color4F _color;
        std::string _url;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<ItemLabel> _label;
        std::shared_ptr<ThumbnailsWidget> _thumbnailsWidget;
        std::shared_ptr<TimeLayout> _markerLayout;
        std::vector<std::shared_ptr<MarkerItem> > _markerItems;

        std::shared_ptr<ftk::ValueObserver<bool> > _thumbnailsObserver;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int border = 0;
        };
        SizeData _size;

        struct GeomData
        {
            ftk::Box2I g2;
            ftk::Box2I g3;
        };
        GeomData _geom;
    };
}
