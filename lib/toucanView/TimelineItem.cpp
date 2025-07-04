// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimelineItem.h"

#include "App.h"
#include "FilesModel.h"
#include "StackItem.h"
#include "WindowModel.h"

#include <feather-tk/ui/ScrollArea.h>

namespace toucan
{
    void TimelineItem::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const ItemData& data,
        const std::shared_ptr<IWidget>& parent)
    {
        IItem::_init(
            context,
            data,
            nullptr,
            data.file->getTimelineWrapper()->getTimeRange(),
            "toucan::TimelineItem",
            parent);

        _setMouseHoverEnabled(true);
        _setMousePressEnabled(
            true,
            0,
            0 | static_cast<int>(feather_tk::KeyModifier::Shift) | static_cast<int>(feather_tk::commandKeyModifier));

        _timeline = data.file->getTimeline();
        _timeRange = data.file->getTimelineWrapper()->getTimeRange();
        _selectionModel = data.file->getSelectionModel();
        _thumbnailGenerator = data.thumbnailGenerator;
        _thumbnailCache = data.thumbnailCache;

        StackItem::create(
            context,
            data,
            _timeline->tracks(),
            _timeline,
            shared_from_this());

        _selectionObserver = feather_tk::ListObserver<SelectionItem>::create(
            _selectionModel->observeSelection(),
            [this](const std::vector<SelectionItem>& selection)
            {
                _select(shared_from_this(), selection);
            });

        _thumbnailsObserver = feather_tk::ValueObserver<bool>::create(
            data.app->getWindowModel()->observeThumbnails(),
            [this](bool value)
            {
                _thumbnails = value;
                _setSizeUpdate();
                _setDrawUpdate();
            });
    }

    TimelineItem::~TimelineItem()
    {}

    std::shared_ptr<TimelineItem> TimelineItem::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const ItemData& data,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<TimelineItem>();
        out->_init(context, data, parent);
        return out;
    }

    const OTIO_NS::RationalTime& TimelineItem::getCurrentTime() const
    {
        return _currentTime;
    }

    void TimelineItem::setCurrentTime(const OTIO_NS::RationalTime& value)
    {
        if (value == _currentTime)
            return;
        _currentTime = value;
        _setDrawUpdate();
    }

    void TimelineItem::setCurrentTimeCallback(const std::function<void(const OTIO_NS::RationalTime&)>& value)
    {
        _currentTimeCallback = value;
    }

    void TimelineItem::setInOutRange(const OTIO_NS::TimeRange& value)
    {
        if (value == _inOutRange)
            return;
        _inOutRange = value;
        _setDrawUpdate();
    }

    void TimelineItem::setGeometry(const feather_tk::Box2I& value)
    {
        IItem::setGeometry(value);
        const feather_tk::Box2I& g = getGeometry();
        const int timeHeight = _size.fontMetrics.lineHeight + _size.margin * 2;
        for (const auto& child : getChildren())
        {
            const feather_tk::Size2I& sizeHint = child->getSizeHint();
            int h = timeHeight;
            if (_thumbnails)
            {
                h += _size.thumbnailHeight;
            }
            child->setGeometry(feather_tk::Box2I(
                g.min.x,
                g.min.y + h,
                sizeHint.w,
                sizeHint.h));
        }
        if (auto scrollArea = getParentT<feather_tk::ScrollArea>())
        {
            _size.scrollPos = scrollArea->getScrollPos();
        }
    }

    void TimelineItem::tickEvent(
        bool parentsVisible,
        bool parentsEnabled,
        const feather_tk::TickEvent& event)
    {
        IItem::tickEvent(parentsVisible, parentsEnabled, event);
        auto i = _thumbnailRequests.begin();
        while (i != _thumbnailRequests.end())
        {
            if (i->future.valid() &&
                i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                const auto image = i->future.get();
                _thumbnailCache->add(getThumbnailCacheKey(nullptr, i->time, _size.thumbnailHeight), image);
                _setDrawUpdate();
                i = _thumbnailRequests.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    void TimelineItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.margin = event.style->getSizeRole(feather_tk::SizeRole::MarginInside, event.displayScale);
            _size.border = event.style->getSizeRole(feather_tk::SizeRole::Border, event.displayScale);
            _size.handle = event.style->getSizeRole(feather_tk::SizeRole::Handle, event.displayScale);
            _size.thumbnailHeight = 2 * event.style->getSizeRole(feather_tk::SizeRole::SwatchLarge, event.displayScale);
            _size.fontInfo = event.style->getFontRole(feather_tk::FontRole::Mono, event.displayScale);
            _size.fontMetrics = event.fontSystem->getMetrics(_size.fontInfo);
            std::vector<uint64_t> ids;
            for (const auto& request : _thumbnailRequests)
            {
                ids.push_back(request.id);
            }
            _thumbnailRequests.clear();
            _thumbnailGenerator->cancelThumbnails(ids);
            _thumbnailCache->clear();
        }
        int childSizeHint = 0;
        for (const auto& child : getChildren())
        {
            childSizeHint = std::max(childSizeHint, child->getSizeHint().h);
        }
        feather_tk::Size2I sizeHint(
            _timeRange.duration().rescaled_to(1.0).value() * _scale,
            _size.fontMetrics.lineHeight + _size.margin * 2);
        if (_thumbnails)
        {
            sizeHint.h += _size.thumbnailHeight;
        }
        sizeHint.h += childSizeHint;
        _setSizeHint(sizeHint);
    }

    void TimelineItem::drawEvent(const feather_tk::Box2I& drawRect, const feather_tk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);

        const feather_tk::Box2I& g = getGeometry();
        const int thumbnailWidth = _size.thumbnailHeight * _thumbnailGenerator->getAspect();
        const int y = g.min.y + _size.fontMetrics.lineHeight + _size.margin * 2;
        if (_thumbnails)
        {
            for (int x = g.min.x; x < g.max.x && thumbnailWidth > 0; x += thumbnailWidth)
            {
                const feather_tk::Box2I g2(x, y, thumbnailWidth, _size.thumbnailHeight);
                if (feather_tk::intersects(g2, drawRect))
                {
                    const OTIO_NS::RationalTime t = posToTime(x);
                    std::shared_ptr<feather_tk::Image> image;
                    if (_thumbnailCache->get(getThumbnailCacheKey(nullptr, t, _size.thumbnailHeight), image))
                    {
                        if (image)
                        {
                            event.render->drawImage(
                                image,
                                feather_tk::Box2I(x, y, image->getWidth(), image->getHeight()));
                        }
                    }
                    else
                    {
                        const auto j = std::find_if(
                            _thumbnailRequests.begin(),
                            _thumbnailRequests.end(),
                            [this, t](const ThumbnailRequest& request)
                            {
                                return t == request.time && _size.thumbnailHeight == request.height;
                            });
                        if (j == _thumbnailRequests.end())
                        {
                            _thumbnailRequests.push_back(
                                _thumbnailGenerator->getThumbnail(t, _size.thumbnailHeight));
                        }
                    }
                }
            }
        }

        std::vector<uint64_t> cancel;
        auto i = _thumbnailRequests.begin();
        while (i != _thumbnailRequests.end())
        {
            const int x = timeToPos(i->time);
            const feather_tk::Box2I g2(x, y, thumbnailWidth, _size.thumbnailHeight);
            if (!feather_tk::intersects(g2, drawRect))
            {
                cancel.push_back(i->id);
                i = _thumbnailRequests.erase(i);
            }
            else
            {
                ++i;
            }
        }
        _thumbnailGenerator->cancelThumbnails(cancel);
    }

    void TimelineItem::drawOverlayEvent(const feather_tk::Box2I& drawRect, const feather_tk::DrawEvent& event)
    {
        IItem::drawOverlayEvent(drawRect, event);
        const feather_tk::Box2I& g = getGeometry();

        const feather_tk::Box2I g2(
            g.min.x,
            g.min.y + _size.scrollPos.y,
            g.w(),
            _size.fontMetrics.lineHeight + _size.margin * 2);
        event.render->drawRect(g2, event.style->getColorRole(feather_tk::ColorRole::Base));

        if (_inOutRange != _timeRange)
        {
            const int x0 = timeToPos(_inOutRange.start_time());
            const int x1 = timeToPos(_inOutRange.end_time_exclusive());
            feather_tk::Color4F color = event.style->getColorRole(feather_tk::ColorRole::Yellow);
            color.a = .5F;
            event.render->drawRect(
                feather_tk::Box2I(
                    x0,
                    g.min.y + _size.scrollPos.y,
                    x1 - x0,
                    _size.fontMetrics.lineHeight + _size.margin * 2),
                color);
        }

        _drawTimeTicks(drawRect, event);
        _drawTimeLabels(drawRect, event);

        int pos = timeToPos(_currentTime);
        event.render->drawRect(
            feather_tk::Box2I(pos, g.min.y + _size.scrollPos.y, _size.border * 2, g.h()),
            event.style->getColorRole(feather_tk::ColorRole::Red));

        std::string s = toString(_currentTime, _timeUnits);
        feather_tk::Size2I size = event.fontSystem->getSize(s, _size.fontInfo);
        feather_tk::Box2I g3(
            pos + _size.border * 2 + _size.margin,
            g.min.y + _size.scrollPos.y + _size.margin,
            size.w,
            _size.fontMetrics.lineHeight);
        if (g3.max.x > g.max.x)
        {
            g3.min.x -= size.w + _size.border * 2 + _size.margin;
            g3.max.x -= size.w + _size.border * 2 + _size.margin;
        }
        event.render->drawText(
            event.fontSystem->getGlyphs(s, _size.fontInfo),
            _size.fontMetrics,
            g3.min,
            event.style->getColorRole(feather_tk::ColorRole::Text));
    }

    void TimelineItem::mouseMoveEvent(feather_tk::MouseMoveEvent& event)
    {
        IItem::mouseMoveEvent(event);
        if (MouseMode::CurrentTime == _mouse.mode)
        {
            _currentTime = posToTime(_getMousePos().x);
            if (_currentTimeCallback)
            {
                _currentTimeCallback(_currentTime);
            }
            _setDrawUpdate();
        }
    }

    void TimelineItem::mousePressEvent(feather_tk::MouseClickEvent& event)
    {
        IItem::mousePressEvent(event);
        if (0 == event.button &&
            (0 == event.modifiers ||
                static_cast<int>(feather_tk::KeyModifier::Shift) == event.modifiers ||
                static_cast<int>(feather_tk::commandKeyModifier) == event.modifiers))
        {
            std::shared_ptr<IItem> selection;
            _select(shared_from_this(), event.pos, selection);
            SelectionItem item;
            if (selection)
            {
                item.object = selection->getObject();
                item.timeRange = selection->getTimeRange();
            }
            if (selection && item.object)
            {
                event.accept = true;
                takeKeyFocus();
                _mouse.mode = MouseMode::Select;
                auto selectionPrev = _selectionModel->getSelection();
                std::vector<SelectionItem> selectionNew;
                if (static_cast<int>(feather_tk::KeyModifier::Shift) == event.modifiers)
                {
                    selectionNew = selectionPrev;
                    selectionNew.insert(selectionNew.end(), item);
                }
                else if (static_cast<int>(feather_tk::commandKeyModifier) == event.modifiers)
                {
                    selectionNew = selectionPrev;
                    auto i = std::find(selectionNew.begin(), selectionNew.end(), item);
                    if (i != selectionNew.end())
                    {
                        selectionNew.erase(i);
                    }
                }
                else if (!selection->isSelected())
                {
                    selectionNew.insert(selectionNew.end(), item);
                }
                _selectionModel->setSelection(selectionNew);
            }
            else if (0 == event.modifiers)
            {
                event.accept = true;
                takeKeyFocus();
                _mouse.mode = MouseMode::CurrentTime;
                _currentTime = posToTime(_getMousePos().x);
                if (_currentTimeCallback)
                {
                    _currentTimeCallback(_currentTime);
                }
                _setDrawUpdate();
            }
        }
    }

    void TimelineItem::mouseReleaseEvent(feather_tk::MouseClickEvent& event)
    {
        IItem::mouseReleaseEvent(event);
        if (_mouse.mode != MouseMode::None)
        {
            event.accept = true;
            _mouse.mode = MouseMode::None;
        }
    }

    void TimelineItem::_timeUnitsUpdate()
    {
        _setSizeUpdate();
        _setDrawUpdate();
    }

    feather_tk::Size2I TimelineItem::_getLabelMaxSize(
        const std::shared_ptr<feather_tk::FontSystem>& fontSystem) const
    {
        const std::string labelMax = toString(_timeRange.end_time_inclusive(), _timeUnits);
        const feather_tk::Size2I labelMaxSize = fontSystem->getSize(labelMax, _size.fontInfo);
        return labelMaxSize;
    }

    void TimelineItem::_getTimeTicks(
        const std::shared_ptr<feather_tk::FontSystem>& fontSystem,
        double& seconds,
        int& tick)
    {
        const int w = getSizeHint().w;
        const float duration = _timeRange.duration().rescaled_to(1.0).value();
        const int secondsTick = 1.0 / duration * w;
        const int minutesTick = 60.0 / duration * w;
        const int hoursTick = 3600.0 / duration * w;
        const feather_tk::Size2I labelMaxSize = _getLabelMaxSize(fontSystem);
        const int distanceMin = _size.border + _size.margin + labelMaxSize.w;
        seconds = 0.0;
        tick = 0;
        if (secondsTick >= distanceMin)
        {
            seconds = 1.0;
            tick = secondsTick;
        }
        else if (minutesTick >= distanceMin)
        {
            seconds = 60.0;
            tick = minutesTick;
        }
        else if (hoursTick >= distanceMin)
        {
            seconds = 3600.0;
            tick = hoursTick;
        }
    }

    void TimelineItem::_drawTimeTicks(
        const feather_tk::Box2I& drawRect,
        const feather_tk::DrawEvent& event)
    {
        if (_timeRange != OTIO_NS::TimeRange())
        {
            const feather_tk::Box2I& g = getGeometry();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            const int frameTick = 1.0 / _timeRange.duration().value() * w;
            if (duration > 0.0 && frameTick >= _size.handle)
            {
                feather_tk::TriMesh2F mesh;
                size_t i = 1;
                const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                const double inc = 1.0 / _timeRange.duration().rate();
                const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                for (double t = x0; t <= x1; t += inc)
                {
                    const feather_tk::Box2I box(
                        g.min.x +
                        t / duration * w,
                        _size.scrollPos.y +
                        g.min.y +
                        _size.fontMetrics.lineHeight,
                        _size.border,
                        _size.margin * 2);
                    if (intersects(box, drawRect))
                    {
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(feather_tk::ColorRole::Button));
                }
            }

            double seconds = 0;
            int tick = 0;
            _getTimeTicks(event.fontSystem, seconds, tick);
            if (duration > 0.0 && seconds > 0.0 && tick > 0)
            {
                feather_tk::TriMesh2F mesh;
                size_t i = 1;
                const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                const double inc = seconds;
                const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                for (double t = x0; t <= x1; t += inc)
                {
                    const feather_tk::Box2I box(
                        g.min.x +
                        t / duration * w,
                        _size.scrollPos.y +
                        g.min.y,
                        _size.border,
                        _size.fontMetrics.lineHeight +
                        _size.margin * 2);
                    if (intersects(box, drawRect))
                    {
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(feather_tk::ColorRole::Button));
                }
            }
        }
    }

    void TimelineItem::_drawTimeLabels(
        const feather_tk::Box2I& drawRect,
        const feather_tk::DrawEvent& event)
    {
        if (_timeRange != OTIO_NS::TimeRange())
        {
            const feather_tk::Box2I& g = getGeometry();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            double seconds = 0;
            int tick = 0;
            _getTimeTicks(event.fontSystem, seconds, tick);
            if (seconds > 0.0 && tick > 0)
            {
                const feather_tk::Size2I labelMaxSize = _getLabelMaxSize(event.fontSystem);
                const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                const double inc = seconds;
                const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                for (double t = x0; t <= x1; t += inc)
                {
                    const OTIO_NS::RationalTime time = _timeRange.start_time() +
                        OTIO_NS::RationalTime(t, 1.0).rescaled_to(_timeRange.duration().rate());
                    const feather_tk::Box2I box(
                        g.min.x +
                        t / duration * w +
                        _size.border +
                        _size.margin,
                        _size.scrollPos.y +
                        g.min.y +
                        _size.margin,
                        labelMaxSize.w,
                        _size.fontMetrics.lineHeight);
                    if (time != _currentTime && intersects(box, drawRect))
                    {
                        const std::string label = toString(time, _timeUnits);
                        event.render->drawText(
                            event.fontSystem->getGlyphs(label, _size.fontInfo),
                            _size.fontMetrics,
                            box.min,
                            event.style->getColorRole(feather_tk::ColorRole::TextDisabled));
                    }
                }
            }
        }
    }

    void TimelineItem::_select(
        const std::shared_ptr<feather_tk::IWidget>& widget,
        const feather_tk::V2I& pos,
        std::shared_ptr<IItem>& out)
    {
        if (auto iitem = std::dynamic_pointer_cast<IItem>(widget))
        {
            if (feather_tk::contains(iitem->getSelectionRect(), pos))
            {
                out = iitem;
            }
        }
        for (const auto& child : widget->getChildren())
        {
            if (feather_tk::contains(child->getGeometry(), pos))
            {
                _select(child, pos, out);
            }
        }
    }

    void TimelineItem::_select(
        const std::shared_ptr<feather_tk::IWidget>& widget,
        const std::vector<SelectionItem>& selection)
    {
        if (auto iitem = std::dynamic_pointer_cast<IItem>(widget))
        {
            auto object = iitem->getObject();
            const auto i = std::find_if(
                selection.begin(),
                selection.end(),
                [object](const SelectionItem& item)
                {
                    return object.value == item.object.value;
                });
            iitem->setSelected(i != selection.end());
        }
        for (const auto& child : widget->getChildren())
        {
            _select(child, selection);
        }
    }
}
