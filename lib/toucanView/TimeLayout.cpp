// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimeLayout.h"

namespace toucan
{
    void ITimeWidget::_init(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::TimeRange& timeRange,
        const std::string& objectName,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::IWidget::_init(context, objectName, parent);
        _timeRange = timeRange;
    }

    ITimeWidget::~ITimeWidget()
    {}

    const OTIO_NS::TimeRange& ITimeWidget::getTimeRange()
    {
        return _timeRange;
    }

    double ITimeWidget::getScale() const
    {
        return _scale;
    }

    void ITimeWidget::setScale(double value)
    {
        if (value == _scale)
            return;
        _scale = value;
        for (const auto& child : getChildren())
        {
            if (auto item = std::dynamic_pointer_cast<ITimeWidget>(child))
            {
                item->setScale(value);
            }
        }
        _setSizeUpdate();
    }

    int ITimeWidget::getMinWidth() const
    {
        return _minWidth;
    }

    OTIO_NS::RationalTime ITimeWidget::posToTime(double value) const
    {
        OTIO_NS::RationalTime out;
        const ftk::Box2I& g = getGeometry();
        if (g.w() > 0)
        {
            const double n = (value - g.min.x) / static_cast<double>(g.w());
            out = OTIO_NS::RationalTime(
                _timeRange.start_time() +
                OTIO_NS::RationalTime(
                    _timeRange.duration().value() * n,
                    _timeRange.duration().rate())).
                round();
            out = ftk::clamp(
                out,
                _timeRange.start_time(),
                _timeRange.end_time_inclusive());
        }
        return out;
    }

    int ITimeWidget::timeToPos(const OTIO_NS::RationalTime& value) const
    {
        int out = 0;
        const ftk::Box2I& g = getGeometry();
        if (_timeRange.duration().value() > 0.0)
        {
            const OTIO_NS::RationalTime t = value - _timeRange.start_time();
            const double n = t.value() / _timeRange.duration().rescaled_to(t).value();
            out = g.min.x + g.w() * n;
        }
        return out;
    }

    void TimeLayout::_init(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ITimeWidget::_init(context, timeRange, "toucan::TimeLayout", parent);
    }

    TimeLayout::~TimeLayout()
    {}

    std::shared_ptr<TimeLayout> TimeLayout::create(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<TimeLayout>(new TimeLayout);
        out->_init(context, timeRange, parent);
        return out;
    }

    void TimeLayout::setGeometry(const ftk::Box2I& value)
    {
        ITimeWidget::setGeometry(value);
        for (const auto& child : getChildren())
        {
            if (auto timeWidget = std::dynamic_pointer_cast<ITimeWidget>(child))
            {
                const OTIO_NS::TimeRange& timeRange = timeWidget->getTimeRange();
                const int t0 = timeToPos(timeRange.start_time());
                const int t1 = timeToPos(timeRange.end_time_exclusive());
                const ftk::Size2I& childSizeHint = child->getSizeHint();
                child->setGeometry(ftk::Box2I(
                    t0,
                    value.min.y,
                    std::max(t1 - t0, timeWidget->getMinWidth()),
                    childSizeHint.h));
            }
        }
    }

    void TimeLayout::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        ITimeWidget::sizeHintEvent(event);
        ftk::Size2I sizeHint;
        for (const auto& child : getChildren())
        {
            if (auto timeWidget = std::dynamic_pointer_cast<ITimeWidget>(child))
            {
                const ftk::Size2I& childSizeHint = timeWidget->getSizeHint();
                sizeHint.h = std::max(sizeHint.h, childSizeHint.h);
            }
        }
        sizeHint.w = _timeRange.duration().rescaled_to(1.0).value() * _scale;
        _setSizeHint(sizeHint);
    }

    void TimeStackLayout::_init(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ITimeWidget::_init(context, timeRange, "toucan::TimeStackLayout", parent);
    }

    TimeStackLayout::~TimeStackLayout()
    {}

    std::shared_ptr<TimeStackLayout> TimeStackLayout::create(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<TimeStackLayout>(new TimeStackLayout);
        out->_init(context, timeRange, parent);
        return out;
    }

    void TimeStackLayout::setGeometry(const ftk::Box2I& value)
    {
        ITimeWidget::setGeometry(value);
        int y = value.min.y;
        for (const auto& child : getChildren())
        {
            if (auto timeWidget = std::dynamic_pointer_cast<ITimeWidget>(child))
            {
                const OTIO_NS::TimeRange& timeRange = timeWidget->getTimeRange();
                const double t0 = timeToPos(timeRange.start_time());
                const double t1 = timeToPos(timeRange.end_time_exclusive());
                const ftk::Size2I& childSizeHint = child->getSizeHint();
                child->setGeometry(ftk::Box2I(t0, y, t1 - t0, childSizeHint.h));
                y += childSizeHint.h + _size.spacing;
            };
        }
    }

    void TimeStackLayout::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        ITimeWidget::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.spacing = event.style->getSizeRole(ftk::SizeRole::SpacingTool, event.displayScale);
        }

        ftk::Size2I sizeHint;
        const auto& children = getChildren();
        if (!children.empty())
        {
            for (const auto& child : children)
            {
                if (auto timeWidget = std::dynamic_pointer_cast<ITimeWidget>(child))
                {
                    const ftk::Size2I& childSizeHint = timeWidget->getSizeHint();
                    sizeHint.h += childSizeHint.h;
                }
            }
            sizeHint.h += (children.size() - 1) * _size.spacing;
        }
        sizeHint.w = _timeRange.duration().rescaled_to(1.0).value() * _scale;
        _setSizeHint(sizeHint);
    }
}
