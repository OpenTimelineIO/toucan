// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "GapItem.h"

#include <dtk/ui/DrawUtil.h>

namespace toucan
{
    void GapItem::_init(
        const std::shared_ptr<dtk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>& gap,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const std::shared_ptr<IWidget>& parent)
    {
        OTIO_NS::TimeRange timeRange = gap->transformed_time_range(
            gap->trimmed_range(),
            timeline->tracks());
        if (timeline->global_start_time().has_value())
        {
            timeRange = OTIO_NS::TimeRange(
                timeline->global_start_time().value() + timeRange.start_time(),
                timeRange.duration());
        }
        IItem::_init(
            context,
            data,
            OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(gap),
            timeRange,
            "toucan::ClipItem",
            parent);

        _gap = gap;
        _text = !gap->name().empty() ? gap->name() : "Gap";
        _color = dtk::Color4F(.3F, .3F, .3F);

        setTooltip(gap->schema_name() + ": " + _text);

        _layout = dtk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(dtk::SizeRole::SpacingTool);

        _label = ItemLabel::create(context, _layout);
        _label->setName(_text);

        const auto& markers = gap->markers();
        if (!markers.empty())
        {
            _markerLayout = TimeLayout::create(context, timeRange, _layout);
            for (const auto& marker : markers)
            {
                OTIO_NS::TimeRange markerTimeRange = gap->transformed_time_range(
                    marker->marked_range(),
                    timeline->tracks());
                if (timeline->global_start_time().has_value())
                {
                    markerTimeRange = OTIO_NS::TimeRange(
                        timeline->global_start_time().value() + markerTimeRange.start_time(),
                        markerTimeRange.duration());
                }
                auto markerItem = MarkerItem::create(
                    context,
                    data,
                    marker,
                    markerTimeRange,
                    _markerLayout);
                _markerItems.push_back(markerItem);
            }
        }

        _textUpdate();
    }
    
    GapItem::~GapItem()
    {}

    std::shared_ptr<GapItem> GapItem::create(
        const std::shared_ptr<dtk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>& gap,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<GapItem>();
        out->_init(context, data, gap, timeline, parent);
        return out;
    }

    void GapItem::setScale(double value)
    {
        IItem::setScale(value);
        if (_markerLayout)
        {
            _markerLayout->setScale(value);
        }
    }

    void GapItem::setGeometry(const dtk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
        _geom.g2 = dtk::margin(value, -_size.border, 0, -_size.border, 0);
        _geom.g3 = dtk::margin(_label->getGeometry(), -_size.border, 0, -_size.border, 0);
        _selectionRect = _geom.g3;
    }

    dtk::Box2I GapItem::getChildrenClipRect() const
    {
        return _geom.g2;
    }

    void GapItem::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
        }
        _setSizeHint(_layout->getSizeHint());
    }

    void GapItem::drawEvent(
        const dtk::Box2I& drawRect,
        const dtk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        event.render->drawRect(
            _geom.g3,
            _selected ? event.style->getColorRole(dtk::ColorRole::Yellow) : _color);
    }

    void GapItem::_timeUnitsUpdate()
    {
        _textUpdate();
    }

    void GapItem::_textUpdate()
    {
        if (_label)
        {
            std::string text = toString(_timeRange.duration(), _timeUnits);
            _label->setDuration(text);
        }
    }
}
