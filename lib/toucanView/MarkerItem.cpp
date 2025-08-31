// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MarkerItem.h"

#include <feather-tk/ui/DrawUtil.h>

namespace toucan
{
    ftk::Color4F getMarkerColor(const std::string& color)
    {
        ftk::Color4F out(1.F, 0.F, 0.F);
        if (color == OTIO_NS::Marker::Color::pink)
        {
            out = ftk::Color4F(1.F, 0.F, .5F);
        }
        else if (color == OTIO_NS::Marker::Color::red)
        {
            out = ftk::Color4F(1.F, 0.F, 0.F);
        }
        else if (color == OTIO_NS::Marker::Color::orange)
        {
            out = ftk::Color4F(1.F, .6F, 0.F);
        }
        else if (color == OTIO_NS::Marker::Color::yellow)
        {
            out = ftk::Color4F(1.F, 1.F, 0.F);
        }
        else if (color == OTIO_NS::Marker::Color::green)
        {
            out = ftk::Color4F(0.F, 1.F, 0.F);
        }
        else if (color == OTIO_NS::Marker::Color::cyan)
        {
            out = ftk::Color4F(0.F, 1.F, 1.F);
        }
        else if (color == OTIO_NS::Marker::Color::blue)
        {
            out = ftk::Color4F(0.F, 0.F, 1.F);
        }
        else if (color == OTIO_NS::Marker::Color::purple)
        {
            out = ftk::Color4F(.5F, 0.F, 1.F);
        }
        else if (color == OTIO_NS::Marker::Color::magenta)
        {
            out = ftk::Color4F(1.F, 0.F, 1.F);
        }
        else if (color == OTIO_NS::Marker::Color::black)
        {
            out = ftk::Color4F(0.F, 0.F, 0.F);
        }
        else if (color == OTIO_NS::Marker::Color::white)
        {
            out = ftk::Color4F(1.F, 1.F, 1.F);
        }
        return out;
    }

    void MarkerItem::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Marker>& marker,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<IWidget>& parent)
    {
        IItem::_init(
            context,
            data,
            OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(marker),
            timeRange,
            "toucan::MarkerItem",
            parent);

        _marker = marker;
        _text = !marker->name().empty() ? marker->name() : "Marker";
        _color = getMarkerColor(marker->color());

        setTooltip(marker->schema_name() + ": " + _text);

        _label = ItemLabel::create(context, shared_from_this());
        _label->setName(_text);

        _textUpdate();
    }
    
    MarkerItem::~MarkerItem()
    {}

    std::shared_ptr<MarkerItem> MarkerItem::create(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Marker>& marker,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<MarkerItem>();
        out->_init(context, data, marker, timeRange, parent);
        return out;
    }

    void MarkerItem::setGeometry(const ftk::Box2I& value)
    {
        IItem::setGeometry(value);
        const ftk::Box2I g(
            value.min.x + value.h(),
            value.min.y,
            value.w() - value.h(),
            value.h());
        _label->setGeometry(g);
    }

    void MarkerItem::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, event.displayScale);
            _size.border = event.style->getSizeRole(ftk::SizeRole::Border, event.displayScale);
        }
        ftk::Size2I sizeHint = _label->getSizeHint();
        sizeHint.h += _size.border * 2;
        _setSizeHint(sizeHint);
        _minWidth = sizeHint.h;
    }

    void MarkerItem::drawEvent(
        const ftk::Box2I& drawRect,
        const ftk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);
        const ftk::Box2I& g = getGeometry();
        const ftk::Box2I g2 = ftk::margin(g, -_size.border, 0, -_size.border, 0);
        event.render->drawRect(
            g2,
            _selected ? event.style->getColorRole(ftk::ColorRole::Yellow) : ftk::Color4F(.3F, .3F, .3F));

        const ftk::Box2I g3(g.min.x, g.min.y, g.h(), g.h());
        event.render->drawMesh(ftk::circle(ftk::center(g3), g3.h() / 4), _color);
    }

    void MarkerItem::_timeUnitsUpdate()
    {
        _textUpdate();
    }

    void MarkerItem::_textUpdate()
    {
        if (_label)
        {
            std::string text = toString(_timeRange.duration(), _timeUnits);
            _label->setDuration(text);
        }
    }
}
