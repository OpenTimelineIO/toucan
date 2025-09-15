// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "StackItem.h"

#include "File.h"
#include "TrackItem.h"

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/core/RenderUtil.h>

namespace toucan
{
    void StackItem::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::Stack* stack,
        const std::shared_ptr<IWidget>& parent)
    {
        auto timelineWrapper = data.file->getTimelineWrapper();
        OTIO_NS::TimeRange timeRange = stack->trimmed_range();
        timeRange = OTIO_NS::TimeRange(
            timelineWrapper->getTimeRange().start_time() + timeRange.start_time(),
            timeRange.duration());
        timeRange = OTIO_NS::TimeRange(
            timeRange.start_time().round(),
            timeRange.duration().round());
        IItem::_init(
            context,
            data,
            stack,
            timeRange,
            "toucan::StackItem",
            parent);

        _stack = stack;
        _text = !stack->name().empty() ? stack->name() : "Stack";
        _color = ftk::Color4F(.2F, .2F, .2F);

        setTooltip(stack->schema_name() + ": " + _text);

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingTool);

        _label = ItemLabel::create(context, _layout);
        _label->setName(_text);

        const auto& markers = stack->markers();
        if (!markers.empty())
        {
            _markerLayout = TimeLayout::create(context, timeRange, _layout);
            for (const auto& marker : markers)
            {
                OTIO_NS::TimeRange markerRange = marker->marked_range();
                markerRange = OTIO_NS::TimeRange(
                    timelineWrapper->getTimeRange().start_time() + markerRange.start_time(),
                    markerRange.duration());
                auto markerItem = MarkerItem::create(
                    context,
                    data,
                    marker,
                    markerRange,
                    _markerLayout);
                _markerItems.push_back(markerItem);
            }
        }

        _timeLayout = TimeStackLayout::create(context, timeRange, _layout);
        for (const auto& child : stack->children())
        {
            if (auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child))
            {
                TrackItem::create(
                    context,
                    data,
                    track,
                    _timeLayout);
            }
        }

        _textUpdate();
    }

    StackItem::~StackItem()
    {}

    std::shared_ptr<StackItem> StackItem::create(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::Stack* stack,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<StackItem>();
        out->_init(context, data, stack, parent);
        return out;
    }

    void StackItem::setScale(double value)
    {
        IItem::setScale(value);
        if (_markerLayout)
        {
            _markerLayout->setScale(value);
        }
        _timeLayout->setScale(value);
    }

    void StackItem::setGeometry(const ftk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
        _geom.g2 = ftk::margin(value, -_size.border, 0, -_size.border, 0);
        _geom.g3 = ftk::margin(_label->getGeometry(), -_size.border, 0, -_size.border, 0);
        _selectionRect = _geom.g3;
    }

    ftk::Box2I StackItem::getChildrenClipRect() const
    {
        return _geom.g2;
    }

    void StackItem::sizeHintEvent(const ftk::SizeHintEvent& event)
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

    void StackItem::drawEvent(
        const ftk::Box2I& drawRect,
        const ftk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        event.render->drawRect(
            _geom.g3,
            _selected ? event.style->getColorRole(ftk::ColorRole::Yellow) : _color);
    }

    void StackItem::_timeUnitsUpdate()
    {
        _textUpdate();
    }

    void StackItem::_textUpdate()
    {
        if (_label)
        {
            std::string text = toString(_timeRange.duration(), _timeUnits);
            _label->setDuration(text);
        }
    }
}
