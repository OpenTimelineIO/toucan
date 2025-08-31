// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "VideoClipItem.h"

#include "App.h"
#include "File.h"
#include "WindowModel.h"

#include <toucanRender/Util.h>

#include <feather-tk/ui/DrawUtil.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

namespace toucan
{
    void VideoClipItem::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>& clip,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const ftk::Color4F& color,
        const std::shared_ptr<IWidget>& parent)
    {
        OTIO_NS::TimeRange timeRange = clip->transformed_time_range(
            clip->trimmed_range(),
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
            OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(clip),
            timeRange,
            "toucan::VideoClipItem",
            parent);

        _clip = clip;
        _text = !clip->name().empty() ? clip->name() : "Video Clip";
        _color = color;

        setTooltip(clip->schema_name() + ": " + _text);

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingTool);

        _label = ItemLabel::create(context, _layout);
        _label->setName(_text);

        _thumbnailsWidget = ThumbnailsWidget::create(
            context,
            _clip->media_reference(),
            data.thumbnailGenerator,
            data.thumbnailCache,
            timeRange,
            _layout);

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
                markerRange = clip->transformed_time_range(markerRange, timeline->tracks());
                if (timeline->global_start_time().has_value())
                {
                    markerRange = OTIO_NS::TimeRange(
                        timeline->global_start_time().value() + markerRange.start_time(),
                        markerRange.duration());
                }
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

        _thumbnailsObserver = ftk::ValueObserver<bool>::create(
            data.app->getWindowModel()->observeThumbnails(),
            [this](bool value)
            {
                _thumbnailsWidget->setVisible(value);
            });
    }
    
    VideoClipItem::~VideoClipItem()
    {}

    std::shared_ptr<VideoClipItem> VideoClipItem::create(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>& clip,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const ftk::Color4F& color,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<VideoClipItem>();
        out->_init(context, data, clip, timeline, color, parent);
        return out;
    }

    void VideoClipItem::setScale(double value)
    {
        IItem::setScale(value);
        if (_markerLayout)
        {
            _markerLayout->setScale(value);
        }
    }

    void VideoClipItem::setGeometry(const ftk::Box2I& value)
    {
        IItem::setGeometry(value);
        _layout->setGeometry(value);
    }

    ftk::Box2I VideoClipItem::getChildrenClipRect() const
    {
        return ftk::margin(getGeometry(), -_size.border, 0, -_size.border, 0);
    }

    void VideoClipItem::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.border = event.style->getSizeRole(ftk::SizeRole::Border, event.displayScale);
        }
        ftk::Size2I sizeHint = _layout->getSizeHint();
        _setSizeHint(sizeHint);
    }
    
    void VideoClipItem::drawEvent(
        const ftk::Box2I& drawRect,
        const ftk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        ftk::Box2I g = _label->getGeometry();
        if (_thumbnailsWidget->isVisible(true))
        {
            const ftk::Box2I& g2 = _thumbnailsWidget->getGeometry();
            g = ftk::Box2I(g.min, g2.max);
        }
        event.render->drawRect(
            ftk::margin(g, -_size.border, 0, -_size.border, 0),
            _selected ? event.style->getColorRole(ftk::ColorRole::Yellow) : _color);
    }

    void VideoClipItem::_timeUnitsUpdate()
    {
        _textUpdate();
    }

    void VideoClipItem::_buildMenu(const std::shared_ptr<ftk::Menu>& menu)
    {
        if (auto externalReference = dynamic_cast<OTIO_NS::ExternalReference*>(_clip->media_reference()))
        {
            auto action = ftk::Action::create(
                "Open Media",
                [this, externalReference]
                {
                    auto file = _file.lock();
                    const std::filesystem::path path = file->getTimelineWrapper()->getMediaPath(externalReference->target_url());
                    auto app = _app.lock();
                    app->open(path);
                });
            menu->addAction(action);
            menu->addDivider();
        }
        else if (auto sequenceRef = dynamic_cast<OTIO_NS::ImageSequenceReference*>(_clip->media_reference()))
        {
            auto action = ftk::Action::create(
                "Open Image Sequence",
                [this, sequenceRef]
                {
                    auto file = _file.lock();
                    const std::string path = getSequenceFrame(
                        file->getTimelineWrapper()->getMediaPath(sequenceRef->target_url_base()),
                        sequenceRef->name_prefix(),
                        sequenceRef->start_frame(),
                        sequenceRef->frame_zero_padding(),
                        sequenceRef->name_suffix());
                    auto app = _app.lock();
                    app->open(path);
                });
            menu->addAction(action);
            menu->addDivider();
        }
        IItem::_buildMenu(menu);
    }

    void VideoClipItem::_textUpdate()
    {
        if (_label)
        {
            std::string text = toString(_timeRange.duration(), _timeUnits);
            _label->setDuration(text);
        }
    }
}
