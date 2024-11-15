// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TrackItem.h"

#include "ClipItem.h"
#include "GapItem.h"

#include <dtk/ui/DrawUtil.h>
#include <dtk/core/RenderUtil.h>

namespace toucan
{
    void TrackItem::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>& track,
        const std::shared_ptr<IWidget>& parent)
    {
        auto opt = track->trimmed_range_in_parent();
        IItem::_init(
            context,
            app,
            OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(track),
            opt.has_value() ? opt.value() : OTIO_NS::TimeRange(),
            "toucan::TrackItem",
            parent);

        _track = track;
        _text = !track->name().empty() ? track->name() : (track->kind() + " Track");
        _color = OTIO_NS::Track::Kind::video == track->kind() ?
            dtk::Color4F(.2F, .2F, .3F) :
            dtk::Color4F(.2F, .3F, .2F);

        setTooltip(_text);

        for (const auto& child : track->children())
        {
            if (auto clip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(child))
            {
                const dtk::Color4F color =
                    OTIO_NS::Track::Kind::video == track->kind() ?
                    dtk::Color4F(.4F, .4F, .6F) :
                    dtk::Color4F(.4F, .6F, .4F);
                ClipItem::create(context, app, clip, color, shared_from_this());
            }
            else if (auto gap = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(child))
            {
                GapItem::create( context, app, gap, shared_from_this());
            }
        }
    }

    TrackItem::~TrackItem()
    {}

    std::shared_ptr<TrackItem> TrackItem::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>& track,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<TrackItem>();
        out->_init(context, app, track, parent);
        return out;
    }

    void TrackItem::setGeometry(const dtk::Box2I& value)
    {
        IItem::setGeometry(value);
        const dtk::Box2I& g = getGeometry();
        const int y = g.min.y + _size.fontMetrics.lineHeight + _size.margin * 2 + _size.border * 2;
        for (const auto& child : getChildren())
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(child))
            {
                const OTIO_NS::TimeRange& timeRange = item->getTimeRange();
                const int x0 = timeToPos(timeRange.start_time());
                const int x1 = timeToPos(timeRange.end_time_exclusive());
                const dtk::Size2I& sizeHint = child->getSizeHint();
                child->setGeometry(dtk::Box2I(x0, y, x1 - x0 + 1, sizeHint.h));
            }
        }
    }

    void TrackItem::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.margin = event.style->getSizeRole(dtk::SizeRole::MarginInside, event.displayScale);
            _size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
            _size.fontInfo = event.style->getFontRole(dtk::FontRole::Label, event.displayScale);
            _size.fontMetrics = event.fontSystem->getMetrics(_size.fontInfo);
            _size.textSize = event.fontSystem->getSize(_text, _size.fontInfo);
            _draw.glyphs.clear();
        }
        dtk::Size2I sizeHint(
            _timeRange.duration().rescaled_to(1.0).value() * _scale,
            0);
        for (const auto& child : getChildren())
        {
            const dtk::Size2I& childSizeHint = child->getSizeHint();
            sizeHint.h = std::max(sizeHint.h, childSizeHint.h);
        }
        sizeHint.h += _size.textSize.h + _size.margin * 2 + _size.border * 4;
        _setSizeHint(sizeHint);
    }

    void TrackItem::clipEvent(const dtk::Box2I& clipRect, bool clipped)
    {
        IItem::clipEvent(clipRect, clipped);
        if (clipped)
        {
            _draw.glyphs.clear();
        }
    }

    void TrackItem::drawEvent(
        const dtk::Box2I& drawRect,
        const dtk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        const dtk::Box2I& g = getGeometry();

        const dtk::Box2I g2(
            g.min.x + _size.border,
            g.min.y,
            g.w() - _size.border * 2,
            _size.fontMetrics.lineHeight + _size.margin * 2);
        event.render->drawRect(
            g2,
            _selected ? event.style->getColorRole(dtk::ColorRole::Yellow) : _color);

        const dtk::Box2I g3 = dtk::margin(g2, -_size.margin);
        if (!_text.empty() && _draw.glyphs.empty())
        {
            _draw.glyphs = event.fontSystem->getGlyphs(_text, _size.fontInfo);
        }
        dtk::ClipRectEnabledState clipRectEnabledState(event.render);
        dtk::ClipRectState clipRectState(event.render);
        event.render->setClipRectEnabled(true);
        event.render->setClipRect(intersect(g3, drawRect));
        event.render->drawText(
            _draw.glyphs,
            _size.fontMetrics,
            g3.min,
            event.style->getColorRole(dtk::ColorRole::Text));
    }
}
