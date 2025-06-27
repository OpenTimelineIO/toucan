// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ItemLabel.h"

namespace toucan
{
    void ItemLabel::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::ItemLabel", parent);
    }

    ItemLabel::~ItemLabel()
    {}

    std::shared_ptr<ItemLabel> ItemLabel::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ItemLabel>(new ItemLabel);
        out->_init(context, parent);
        return out;
    }

    void ItemLabel::setName(const std::string& value)
    {
        if (value == _name)
            return;
        _name = value;
        _setSizeUpdate();
        _setDrawUpdate();
    }

    void ItemLabel::setDuration(const std::string& value)
    {
        if (value == _duration)
            return;
        _duration = value;
        _setSizeUpdate();
        _setDrawUpdate();
    }

    void ItemLabel::setMarginRole(feather_tk::SizeRole value)
    {
        if (value == _marginRole)
            return;
        _marginRole = value;
        _setSizeUpdate();
        _setDrawUpdate();
    }

    void ItemLabel::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
    }

    void ItemLabel::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.margin = event.style->getSizeRole(_marginRole, event.displayScale);
            _size.margin2 = event.style->getSizeRole(feather_tk::SizeRole::MarginInside, event.displayScale);
            _size.fontInfo = event.style->getFontRole(feather_tk::FontRole::Label, event.displayScale);
            _size.fontMetrics = event.fontSystem->getMetrics(_size.fontInfo);
            _size.nameSize = event.fontSystem->getSize(_name, _size.fontInfo);
            _size.durationSize = event.fontSystem->getSize(_duration, _size.fontInfo);
            _draw.nameGlyphs.clear();
            _draw.durationGlyphs.clear();
        }
        feather_tk::Size2I sizeHint;
        sizeHint.w =
            _size.nameSize.w + _size.margin2 * 2 +
            _size.durationSize.w + _size.margin2 * 2 +
            _size.margin * 2;
        sizeHint.h = std::max(_size.nameSize.h, _size.durationSize.h) + _size.margin * 2;
        _setSizeHint(sizeHint);
    }

    void ItemLabel::clipEvent(const feather_tk::Box2I& clipRect, bool clipped)
    {
        IWidget::clipEvent(clipRect, clipped);
        if (clipped)
        {
            _draw.nameGlyphs.clear();
            _draw.durationGlyphs.clear();
        }
    }

    void ItemLabel::drawEvent(const feather_tk::Box2I& drawRect, const feather_tk::DrawEvent& event)
    {
        IWidget::drawEvent(drawRect, event);
        const feather_tk::Box2I& g = getGeometry();
        const feather_tk::Box2I g2 = margin(g, -_size.margin);

        int w = _size.nameSize.w + _size.margin * 2;
        int h = _size.fontMetrics.lineHeight;
        const feather_tk::Box2I g3(
            g2.min.x,
            g2.min.y + g2.h() / 2 - h / 2,
            w,
            h);
        if (!_name.empty() && _draw.nameGlyphs.empty())
        {
            _draw.nameGlyphs = event.fontSystem->getGlyphs(_name, _size.fontInfo);
        }
        event.render->drawText(
            _draw.nameGlyphs,
            _size.fontMetrics,
            feather_tk::V2I(g3.min.x + _size.margin2, g3.min.y),
            event.style->getColorRole(feather_tk::ColorRole::Text));

        w = _size.durationSize.w + _size.margin2 * 2;
        const feather_tk::Box2I g4(
            g2.max.x - w,
            g2.min.y + g2.h() / 2 - h / 2,
            w,
            h);
        if (!feather_tk::intersects(g4, g3))
        {
            if (!_duration.empty() && _draw.durationGlyphs.empty())
            {
                _draw.durationGlyphs = event.fontSystem->getGlyphs(_duration, _size.fontInfo);
            }
            event.render->drawText(
                _draw.durationGlyphs,
                _size.fontMetrics,
                feather_tk::V2I(g4.min.x + _size.margin2, g4.min.y),
                event.style->getColorRole(feather_tk::ColorRole::Text));
        }
    }
}
