// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IItem.h>
#include <toucanView/ItemLabel.h>

#include <feather-tk/ui/RowLayout.h>

#include <opentimelineio/marker.h>

namespace toucan
{
    //! Get a marker color.
    feather_tk::Color4F getMarkerColor(const std::string&);

    //! Timeline marker item.
    class MarkerItem : public IItem
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const ItemData&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Marker>&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~MarkerItem();

        //! Create a new item.
        static std::shared_ptr<MarkerItem> create(
            const std::shared_ptr<feather_tk::Context>&,
            const ItemData&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Marker>&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

    protected:
        void _timeUnitsUpdate() override;

    private:
        void _textUpdate();

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Marker> _marker;
        std::string _text;
        feather_tk::Color4F _color;

        std::shared_ptr<ItemLabel> _label;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int margin = 0;
            int border = 0;
        };
        SizeData _size;
    };
}
