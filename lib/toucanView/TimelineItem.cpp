// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimelineItem.h"

#include "App.h"
#include "DocumentsModel.h"
#include "SelectionModel.h"
#include "StackItem.h"

#include <dtk/ui/ScrollArea.h>

namespace toucan
{
    void TimelineItem::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<Document>& document,
        const std::shared_ptr<IWidget>& parent)
    {
        IItem::_init(
            context,
            app,
            nullptr,
            document->getTimeline()->getTimeRange(),
            "toucan::TimelineItem",
            parent);

        _setMouseHoverEnabled(true);
        _setMousePressEnabled(
            true,
            0,
            0 | static_cast<int>(dtk::KeyModifier::Shift) | static_cast<int>(dtk::KeyModifier::Control));

        _timeline = document->getTimeline()->otio();
        _timeUnitsModel = app->getTimeUnitsModel();
        _selectionModel = document->getSelectionModel();
        _thumbnails.setMax(100);
        _thumbnailGenerator = document->getThumbnailGenerator();

        StackItem::create(context, app, _timeline->tracks(), shared_from_this());

        _timeUnitsObserver = dtk::ValueObserver<TimeUnits>::create(
            _timeUnitsModel->observeTimeUnits(),
            [this](TimeUnits)
            {
                _setDrawUpdate();
            });

        _selectionObserver = dtk::ListObserver<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item> >::create(
            _selectionModel->observeSelection(),
            [this](const std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item> >& selection)
            {
                _select(shared_from_this(), selection);
            });

    }

    TimelineItem::~TimelineItem()
    {}

    std::shared_ptr<TimelineItem> TimelineItem::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<Document>& document,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<TimelineItem>();
        out->_init(context, app, document, parent);
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

    void TimelineItem::setGeometry(const dtk::Box2I& value)
    {
        IItem::setGeometry(value);
        const dtk::Box2I& g = getGeometry();
        const int timeHeight = _size.fontMetrics.lineHeight + _size.margin * 2;
        for (const auto& child : getChildren())
        {
            const dtk::Size2I& sizeHint = child->getSizeHint();
            child->setGeometry(dtk::Box2I(
                g.min.x,
                g.min.y + timeHeight + _size.thumbnailSize.h,
                sizeHint.w,
                sizeHint.h));
        }
        if (auto scrollArea = getParentT<dtk::ScrollArea>())
        {
            _size.scrollPos = scrollArea->getScrollPos();
        }
    }

    void TimelineItem::tickEvent(
        bool parentsVisible,
        bool parentsEnabled,
        const dtk::TickEvent& event)
    {
        IItem::tickEvent(parentsVisible, parentsEnabled, event);
        auto i = _thumbnailRequests.begin();
        while (i != _thumbnailRequests.end())
        {
            if (i->future.valid() &&
                i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                const auto image = i->future.get();
                _thumbnails.add(i->time, image);
                _setDrawUpdate();
                i = _thumbnailRequests.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    void TimelineItem::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IItem::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.margin = event.style->getSizeRole(dtk::SizeRole::MarginInside, event.displayScale);
            _size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
            _size.handle = event.style->getSizeRole(dtk::SizeRole::Handle, event.displayScale);
            _size.thumbnailSize.h = 2 * event.style->getSizeRole(dtk::SizeRole::SwatchLarge, event.displayScale);
            _size.thumbnailSize.w = _size.thumbnailSize.h * _thumbnailGenerator->getAspect();
            _size.fontInfo = event.style->getFontRole(dtk::FontRole::Label, event.displayScale);
            _size.fontMetrics = event.fontSystem->getMetrics(_size.fontInfo);
            std::vector<uint64_t> ids;
            for (const auto& request : _thumbnailRequests)
            {
                ids.push_back(request.id);
            }
            _thumbnailRequests.clear();
            _thumbnailGenerator->cancelThumbnails(ids);
            _thumbnails.clear();
        }
        int childSizeHint = 0;
        for (const auto& child : getChildren())
        {
            childSizeHint = std::max(childSizeHint, child->getSizeHint().h);
        }
        dtk::Size2I sizeHint(
            _timeRange.duration().rescaled_to(1.0).value() * _scale,
            _size.fontMetrics.lineHeight + _size.margin * 2);
        sizeHint.h += _size.thumbnailSize.h;
        sizeHint.h += childSizeHint;
        _setSizeHint(sizeHint);
    }

    void TimelineItem::drawEvent(const dtk::Box2I& drawRect, const dtk::DrawEvent& event)
    {
        IItem::drawEvent(drawRect, event);

        const dtk::Box2I& g = getGeometry();
        const int y = g.min.y + _size.fontMetrics.lineHeight + _size.margin * 2;
        for (int x = g.min.x; x < g.max.x && _size.thumbnailSize.w > 0; x += _size.thumbnailSize.w)
        {
            const dtk::Box2I g2(x, y, _size.thumbnailSize.w, _size.thumbnailSize.h);
            if (dtk::intersects(g2, drawRect))
            {
                const OTIO_NS::RationalTime t = posToTime(x);
                std::shared_ptr<dtk::Image> image;
                if (_thumbnails.get(t, image))
                {
                    if (image)
                    {
                        event.render->drawImage(
                            image,
                            dtk::Box2I(x, y, image->getWidth(), image->getHeight()));
                    }
                }
                else
                {
                    const auto j = std::find_if(
                        _thumbnailRequests.begin(),
                        _thumbnailRequests.end(),
                        [this, t](const ThumbnailRequest& request)
                        {
                            return t == request.time && _size.thumbnailSize.h == request.height;
                        });
                    if (j == _thumbnailRequests.end())
                    {
                        _thumbnailRequests.push_back(
                            _thumbnailGenerator->getThumbnail(t, _size.thumbnailSize.h));
                    }
                }
            }
        }

        std::vector<uint64_t> cancel;
        auto i = _thumbnailRequests.begin();
        while (i != _thumbnailRequests.end())
        {
            const int x = timeToPos(i->time);
            const dtk::Box2I g2(x, y, _size.thumbnailSize.w, _size.thumbnailSize.h);
            if (!dtk::intersects(g2, drawRect))
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

    void TimelineItem::drawOverlayEvent(const dtk::Box2I& drawRect, const dtk::DrawEvent& event)
    {
        IItem::drawOverlayEvent(drawRect, event);
        const dtk::Box2I& g = getGeometry();

        const dtk::Box2I g2(
            g.min.x,
            g.min.y + _size.scrollPos.y,
            g.w(),
            _size.fontMetrics.lineHeight + _size.margin * 2);
        event.render->drawRect(g2, event.style->getColorRole(dtk::ColorRole::Base));

        _drawTimeTicks(drawRect, event);
        _drawTimeLabels(drawRect, event);

        int pos = timeToPos(_currentTime);
        event.render->drawRect(
            dtk::Box2I(pos, g.min.y + _size.scrollPos.y, _size.border * 2, g.h()),
            event.style->getColorRole(dtk::ColorRole::Red));

        std::string s = _timeUnitsModel->getLabel(_currentTime);
        dtk::Size2I size = event.fontSystem->getSize(s, _size.fontInfo);
        dtk::Box2I g3(
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
            event.style->getColorRole(dtk::ColorRole::Text));
    }

    void TimelineItem::mouseMoveEvent(dtk::MouseMoveEvent& event)
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

    void TimelineItem::mousePressEvent(dtk::MouseClickEvent& event)
    {
        IItem::mousePressEvent(event);
        if (0 == event.button &&
            (0 == event.modifiers ||
                static_cast<int>(dtk::KeyModifier::Shift) == event.modifiers ||
                static_cast<int>(dtk::KeyModifier::Control) == event.modifiers))
        {
            event.accept = true;
            auto selection = _select(shared_from_this(), event.pos);
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item> item;
            if (selection)
            {
                item = selection->getItem();
            }
            if (selection && item)
            {
                _mouse.mode = MouseMode::Select;
                auto selectionPrev = _selectionModel->getSelection();
                std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item> > selectionNew;
                if (static_cast<int>(dtk::KeyModifier::Shift) == event.modifiers)
                {
                    selectionNew = selectionPrev;
                    selectionNew.insert(selectionNew.end(), item);
                }
                else if (static_cast<int>(dtk::KeyModifier::Control) == event.modifiers)
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
            else
            {
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

    void TimelineItem::mouseReleaseEvent(dtk::MouseClickEvent& event)
    {
        IItem::mouseReleaseEvent(event);
        if (_mouse.mode != MouseMode::None)
        {
            event.accept = true;
            _mouse.mode = MouseMode::None;
        }
    }

    dtk::Size2I TimelineItem::_getLabelMaxSize(
        const std::shared_ptr<dtk::FontSystem>& fontSystem) const
    {
        const std::string labelMax = _timeUnitsModel->getLabel(_timeRange.duration());
        const dtk::Size2I labelMaxSize = fontSystem->getSize(labelMax, _size.fontInfo);
        return labelMaxSize;
    }

    void TimelineItem::_getTimeTicks(
        const std::shared_ptr<dtk::FontSystem>& fontSystem,
        double& seconds,
        int& tick)
    {
        const int w = getSizeHint().w;
        const float duration = _timeRange.duration().rescaled_to(1.0).value();
        const int secondsTick = 1.0 / duration * w;
        const int minutesTick = 60.0 / duration * w;
        const int hoursTick = 3600.0 / duration * w;
        const dtk::Size2I labelMaxSize = _getLabelMaxSize(fontSystem);
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
        const dtk::Box2I& drawRect,
        const dtk::DrawEvent& event)
    {
        if (_timeRange != OTIO_NS::TimeRange())
        {
            const dtk::Box2I& g = getGeometry();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            const int frameTick = 1.0 / _timeRange.duration().value() * w;
            if (duration > 0.0 && frameTick >= _size.handle)
            {
                dtk::TriMesh2F mesh;
                size_t i = 1;
                const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                const double inc = 1.0 / _timeRange.duration().rate();
                const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                for (double t = x0; t <= x1; t += inc)
                {
                    const dtk::Box2I box(
                        g.min.x +
                        t / duration * w,
                        _size.scrollPos.y +
                        g.min.y +
                        _size.fontMetrics.lineHeight,
                        _size.border,
                        _size.margin * 2);
                    if (intersects(box, drawRect))
                    {
                        mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(dtk::ColorRole::Button));
                }
            }

            double seconds = 0;
            int tick = 0;
            _getTimeTicks(event.fontSystem, seconds, tick);
            if (duration > 0.0 && seconds > 0.0 && tick > 0)
            {
                dtk::TriMesh2F mesh;
                size_t i = 1;
                const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                const double inc = seconds;
                const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                for (double t = x0; t <= x1; t += inc)
                {
                    const dtk::Box2I box(
                        g.min.x +
                        t / duration * w,
                        _size.scrollPos.y +
                        g.min.y,
                        _size.border,
                        _size.fontMetrics.lineHeight +
                        _size.margin * 2);
                    if (intersects(box, drawRect))
                    {
                        mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(dtk::ColorRole::Button));
                }
            }
        }
    }

    void TimelineItem::_drawTimeLabels(
        const dtk::Box2I& drawRect,
        const dtk::DrawEvent& event)
    {
        if (_timeRange != OTIO_NS::TimeRange())
        {
            const dtk::Box2I& g = getGeometry();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            double seconds = 0;
            int tick = 0;
            _getTimeTicks(event.fontSystem, seconds, tick);
            if (seconds > 0.0 && tick > 0)
            {
                const dtk::Size2I labelMaxSize = _getLabelMaxSize(event.fontSystem);
                const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                const double inc = seconds;
                const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                for (double t = x0; t <= x1; t += inc)
                {
                    const OTIO_NS::RationalTime time = _timeRange.start_time() +
                        OTIO_NS::RationalTime(t, 1.0).rescaled_to(_timeRange.duration().rate());
                    const dtk::Box2I box(
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
                        const std::string label = _timeUnitsModel->getLabel(time);
                        event.render->drawText(
                            event.fontSystem->getGlyphs(label, _size.fontInfo),
                            _size.fontMetrics,
                            box.min,
                            event.style->getColorRole(dtk::ColorRole::TextDisabled));
                    }
                }
            }
        }
    }

    std::shared_ptr<IItem> TimelineItem::_select(
        const std::shared_ptr<dtk::IWidget>& widget,
        const dtk::V2I& pos)
    {
        std::shared_ptr<IItem> out;
        if (auto iitem = std::dynamic_pointer_cast<IItem>(widget))
        {
            out = iitem;
        }
        for (const auto& child : widget->getChildren())
        {
            if (dtk::contains(child->getGeometry(), pos))
            {
                out = _select(child, pos);
                break;
            }
        }
        return out;
    }

    void TimelineItem::_select(
        const std::shared_ptr<dtk::IWidget>& widget,
        const std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item> >& selection)
    {
        if (auto iitem = std::dynamic_pointer_cast<IItem>(widget))
        {
            const auto i = std::find(selection.begin(), selection.end(), iitem->getItem());
            iitem->setSelected(i != selection.end());
        }
        for (const auto& child : widget->getChildren())
        {
            _select(child, selection);
        }
    }
}
