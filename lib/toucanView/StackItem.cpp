// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "StackItem.h"

#include "TrackItem.h"

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/core/RenderUtil.h>

namespace toucan
{
    void StackItem::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline ,
        const std::shared_ptr<IWidget>& parent)
    {
        OTIO_NS::TimeRange timeRange = stack->trimmed_range();
        if (timeline->global_start_time().has_value())
        {
            timeRange = OTIO_NS::TimeRange(
                timeline->global_start_time().value() + timeRange.start_time(),
                timeRange.duration());
        }
        timeRange = OTIO_NS::TimeRange(
            timeRange.start_time().round(),
            timeRange.duration().round());
        IItem::_init(
            context,
            data,
            OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(stack),
            timeRange,
            "toucan::StackItem",
            parent);

        _stack = stack;
        _text = !stack->name().empty() ? stack->name() : "Stack";
        _color = feather_tk::Color4F(.2F, .2F, .2F);

        setTooltip(stack->schema_name() + ": " + _text);

        _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        _label = ItemLabel::create(context, _layout);
        _label->setName(_text);

        const auto& markers = stack->markers();
        if (!markers.empty())
        {
            _markerLayout = TimeLayout::create(context, timeRange, _layout);
            for (const auto& marker : markers)
            {
                OTIO_NS::TimeRange markerTimeRange = marker->marked_range();
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

        _timeLayout = TimeStackLayout::create(context, timeRange, _layout);
        for (const auto& child : stack->children())
        {
            if (auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child))
            {
                TrackItem::create(
                    context,
                    data,
                    track,
                    timeline,
                    _timeLayout);
            }
        }

        _textUpdate();
    }

    StackItem::~StackItem()
    {}

    std::shared_ptr<StackItem> StackItem::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<StackItem>();
        out->_init(context, data, stack, timeline, parent);
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

    void StackItem::setGeometry(const feather_tk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
        _geom.g2 = feather_tk::margin(value, -_size.border, 0, -_size.border, 0);
        _geom.g3 = feather_tk::margin(_label->getGeometry(), -_size.border, 0, -_size.border, 0);
        _selectionRect = _geom.g3;
    }

    feather_tk::Box2I StackItem::getChildrenClipRect() const
    {
        return _geom.g2;
    }

    void StackItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.border = event.style->getSizeRole(feather_tk::SizeRole::Border, event.displayScale);
        }
        _setSizeHint(_layout->getSizeHint());
    }

    void StackItem::drawEvent(
        const feather_tk::Box2I& drawRect,
        const feather_tk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        event.render->drawRect(
            _geom.g3,
            _selected ? event.style->getColorRole(feather_tk::ColorRole::Yellow) : _color);
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
