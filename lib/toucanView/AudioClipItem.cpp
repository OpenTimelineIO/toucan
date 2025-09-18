// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "AudioClipItem.h"

#include "App.h"
#include "File.h"

#include <toucanRender/Util.h>

#include <feather-tk/ui/DrawUtil.h>

#include <opentimelineio/externalReference.h>

namespace toucan
{
    void AudioClipItem::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::Clip* clip,
        const ftk::Color4F& color,
        const std::shared_ptr<IWidget>& parent)
    {
        auto timelineWrapper = data.file->getTimelineWrapper();
        OTIO_NS::TimeRange timeRange = clip->transformed_time_range(
            clip->trimmed_range(),
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
            clip,
            timeRange,
            "toucan::AudioClipItem",
            parent);

        _clip = clip;
        _text = !clip->name().empty() ? clip->name() : "Audio Clip";
        _color = color;

        setTooltip(clip->schema_name() + ": " + _text);

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingTool);

        _label = ItemLabel::create(context, _layout);
        _label->setName(_text);

        const auto& markers = clip->markers();
        if (!markers.empty())
        {
            _markerLayout = TimeLayout::create(context, timeRange, _layout);
            for (const auto& marker : markers)
            {
                const OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                OTIO_NS::TimeRange markerRange(
                    marker->marked_range().start_time() + trimmedRange.start_time(),
                    marker->marked_range().duration());
                markerRange = clip->transformed_time_range(markerRange, timelineWrapper->getTimeline()->tracks());
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

        _textUpdate();
    }
    
    AudioClipItem::~AudioClipItem()
    {}

    std::shared_ptr<AudioClipItem> AudioClipItem::create(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::Clip* clip,
        const ftk::Color4F& color,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<AudioClipItem>();
        out->_init(context, data, clip, color, parent);
        return out;
    }

    void AudioClipItem::setScale(double value)
    {
        IItem::setScale(value);
        if (_markerLayout)
        {
            _markerLayout->setScale(value);
        }
    }

    void AudioClipItem::setGeometry(const ftk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
        _geom.g2 = ftk::margin(value, -_size.border, 0, -_size.border, 0);
        _geom.g3 = ftk::margin(_label->getGeometry(), -_size.border, 0, -_size.border, 0);
    }

    ftk::Box2I AudioClipItem::getChildrenClipRect() const
    {
        return _geom.g2;
    }

    void AudioClipItem::sizeHintEvent(const ftk::SizeHintEvent& event)
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
    
    void AudioClipItem::drawEvent(
        const ftk::Box2I& drawRect,
        const ftk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        event.render->drawRect(
            _geom.g3,
            _selected ? event.style->getColorRole(ftk::ColorRole::Yellow) : _color);
    }

    void AudioClipItem::_timeUnitsUpdate()
    {
        _textUpdate();
    }

    void AudioClipItem::_buildMenu(const std::shared_ptr<ftk::Menu>& menu)
    {
        if (auto externalReference = dynamic_cast<OTIO_NS::ExternalReference*>(_clip->media_reference()))
        {
            auto action = ftk::Action::create(
                "Open Media",
                [this, externalReference]
                {
                    const std::filesystem::path path = _file->getTimelineWrapper()->getMediaPath(externalReference->target_url());
                    auto app = _app.lock();
                    app->open(path);
                });
            menu->addAction(action);
            menu->addDivider();
        }
        IItem::_buildMenu(menu);
    }

    void AudioClipItem::_textUpdate()
    {
        if (_label)
        {
            std::string text = toString(_timeRange.duration(), _timeUnits);
            _label->setDuration(text);
        }
    }
}
