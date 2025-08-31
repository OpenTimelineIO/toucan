// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/MarkerItem.h>

#include <feather-tk/ui/RowLayout.h>

#include <opentimelineio/track.h>

namespace toucan
{
    //! Timeline track item.
    class TrackItem : public IItem
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const ItemData&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TrackItem();

        //! Create a new item.
        static std::shared_ptr<TrackItem> create(
            const std::shared_ptr<ftk::Context>&,
            const ItemData&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setScale(double) override;

        void setGeometry(const ftk::Box2I&) override;
        ftk::Box2I getChildrenClipRect() const override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

    protected:
        void _timeUnitsUpdate() override;

    private:
        void _textUpdate();

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track> _track;
        std::string _text;
        ftk::Color4F _color;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<ItemLabel> _label;
        std::shared_ptr<TimeLayout> _markerLayout;
        std::vector<std::shared_ptr<MarkerItem> > _markerItems;
        std::shared_ptr<TimeLayout> _timeLayout;

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
