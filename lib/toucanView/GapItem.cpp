// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "GapItem.h"

#include "File.h"

#include <feather-tk/ui/DrawUtil.h>

namespace toucan
{
    void GapItem::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::Gap* gap,
        const std::shared_ptr<IWidget>& parent)
    {
        auto timelineWrapper = data.file->getTimelineWrapper();
        OTIO_NS::TimeRange timeRange = gap->transformed_time_range(
            gap->trimmed_range(),
            timelineWrapper->getTimeline()->tracks());
        timeRange = OTIO_NS::TimeRange(
            timelineWrapper->getTimeRange().start_time() + timeRange.start_time(),
            timeRange.duration());
        timeRange = OTIO_NS::TimeRange(
            timeRange.start_time().round(),
            timeRange.duration().round());
        IItem::_init(
            context,
            data,
            gap,
            timeRange,
            "toucan::ClipItem",
            parent);

        _gap = gap;
        _text = !gap->name().empty() ? gap->name() : "Gap";
        _color = ftk::Color4F(.3F, .3F, .3F);

        setTooltip(gap->schema_name() + ": " + _text);

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingTool);

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
                    timelineWrapper->getTimeline()->tracks());
                markerTimeRange = OTIO_NS::TimeRange(
                    timelineWrapper->getTimeRange().start_time() + markerTimeRange.start_time(),
                    markerTimeRange.duration());
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
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::Gap* gap,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<GapItem>();
        out->_init(context, data, gap, parent);
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

    void GapItem::setGeometry(const ftk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
        _geom.g2 = ftk::margin(value, -_size.border, 0, -_size.border, 0);
        _geom.g3 = ftk::margin(_label->getGeometry(), -_size.border, 0, -_size.border, 0);
        _selectionRect = _geom.g3;
    }

    ftk::Box2I GapItem::getChildrenClipRect() const
    {
        return _geom.g2;
    }

    void GapItem::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.border = event.style->getSizeRole(ftk::SizeRole::Border, event.displayScale);
        }
        _setSizeHint(_layout->getSizeHint());
    }

    void GapItem::drawEvent(
        const ftk::Box2I& drawRect,
        const ftk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        event.render->drawRect(
            _geom.g3,
            _selected ? event.style->getColorRole(ftk::ColorRole::Yellow) : _color);
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
