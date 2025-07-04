// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TrackItem.h"

#include "AudioClipItem.h"
#include "GapItem.h"
#include "VideoClipItem.h"

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/core/RenderUtil.h>

namespace toucan
{
    void TrackItem::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>& track,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const std::shared_ptr<IWidget>& parent)
    {
        OTIO_NS::TimeRange timeRange = track->transformed_time_range(
            track->trimmed_range(),
            timeline->tracks());
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
            OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(track),
            timeRange,
            "toucan::TrackItem",
            parent);

        _track = track;
        _text = !track->name().empty() ? track->name() : (track->kind() + " Track");
        _color = OTIO_NS::Track::Kind::video == track->kind() ?
            feather_tk::Color4F(.2F, .2F, .3F) :
            feather_tk::Color4F(.2F, .3F, .2F);

        setTooltip(track->schema_name() + ": " + _text);

        _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        _label = ItemLabel::create(context, _layout);
        _label->setName(_text);

        const auto& markers = track->markers();
        if (!markers.empty())
        {
            _markerLayout = TimeLayout::create(context, timeRange, _layout);
            for (const auto& marker : markers)
            {
                OTIO_NS::TimeRange markerTimeRange = track->transformed_time_range(
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

        _timeLayout = TimeLayout::create(context, timeRange, _layout);
        for (const auto& child : track->children())
        {
            if (auto clip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(child))
            {
                if (OTIO_NS::Track::Kind::video == track->kind())
                {
                    VideoClipItem::create(
                        context,
                        data,
                        clip,
                        timeline,
                        feather_tk::Color4F(.4F, .4F, .6F),
                        _timeLayout);
                }
                else if (OTIO_NS::Track::Kind::audio == track->kind())
                {
                    AudioClipItem::create(
                        context,
                        data,
                        clip,
                        timeline,
                        feather_tk::Color4F(.4F, .6F, .4F),
                        _timeLayout);
                }
            }
            else if (auto gap = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(child))
            {
                GapItem::create(
                    context,
                    data,
                    gap,
                    timeline,
                    _timeLayout);
            }
        }

        _textUpdate();
    }

    TrackItem::~TrackItem()
    {}

    std::shared_ptr<TrackItem> TrackItem::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>& track,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<TrackItem>();
        out->_init(context, data, track, timeline, parent);
        return out;
    }

    void TrackItem::setScale(double value)
    {
        IItem::setScale(value);
        if (_markerLayout)
        {
            _markerLayout->setScale(value);
        }
        _timeLayout->setScale(value);
    }

    void TrackItem::setGeometry(const feather_tk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
        _geom.g2 = feather_tk::margin(value, -_size.border, 0, -_size.border, 0);
        _geom.g3 = feather_tk::margin(_label->getGeometry(), -_size.border, 0, -_size.border, 0);
        _selectionRect = _geom.g3;
    }

    feather_tk::Box2I TrackItem::getChildrenClipRect() const
    {
        return _geom.g2;
    }

    void TrackItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
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

    void TrackItem::drawEvent(
        const feather_tk::Box2I& drawRect,
        const feather_tk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        event.render->drawRect(
            _geom.g3,
            _selected ? event.style->getColorRole(feather_tk::ColorRole::Yellow) : _color);
    }

    void TrackItem::_timeUnitsUpdate()
    {
        _textUpdate();
    }

    void TrackItem::_textUpdate()
    {
        if (_label)
        {
            std::string text = toString(_timeRange.duration(), _timeUnits);
            _label->setDuration(text);
        }
    }
}
