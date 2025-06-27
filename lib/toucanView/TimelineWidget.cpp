// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimelineWidget.h"

#include "App.h"
#include "FilesModel.h"
#include "TimelineItem.h"
#include "WindowModel.h"

namespace toucan
{
    namespace
    {
        const float marginPercentage = .1F;
    }

    void TimelineWidget::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<IWidget>& parent)
    {
        IWidget::_init(context, "toucan::TimelineWidget", parent);

        _setMouseHoverEnabled(true);
        _setMousePressEnabled(
            true,
            0,
            static_cast<int>(feather_tk::KeyModifier::Alt));

        _frameView = feather_tk::ObservableValue<bool>::create(true);

        _scrollWidget = feather_tk::ScrollWidget::create(context, feather_tk::ScrollType::Both, shared_from_this());
        _scrollWidget->setScrollEventsEnabled(false);
        _scrollWidget->setBorder(false);

        auto appWeak = std::weak_ptr<App>(app);
        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this, appWeak](const std::shared_ptr<File>& file)
            {
                if (_file)
                {
                    TimelineViewState viewState;
                    viewState.pos = _scrollWidget->getScrollPos();
                    viewState.scale = _scale;
                    viewState.frameView = _frameView->get();
                    _file->getPlaybackModel()->setViewState(viewState);
                }
                _file = file;
                if (file)
                {
                    _timeRange = file->getPlaybackModel()->getTimeRange();
                    _viewState = file->getPlaybackModel()->getViewState();
                    if (_viewState.has_value())
                    {
                        _scale = _viewState->scale;
                    }
                    else
                    {
                        _sizeInit = true;
                    }

                    ItemData data;
                    data.app = appWeak.lock();
                    data.file = file;
                    data.thumbnailGenerator = file->getThumbnailGenerator();
                    data.thumbnailCache = std::make_shared<feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > >();
                    data.thumbnailCache->setMax(1000);
                    _timelineItem = TimelineItem::create(getContext(), data);
                    _timelineItem->setScale(_scale);
                    _timelineItem->setCurrentTimeCallback(
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            if (_file)
                            {
                                _file->getPlaybackModel()->setCurrentTime(value, CurrentTime::Free);
                            }
                        });
                    _scrollWidget->setWidget(_timelineItem);

                    _currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                        file->getPlaybackModel()->observeCurrentTime(),
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            _currentTime = value;
                            if (_timelineItem)
                            {
                                _timelineItem->setCurrentTime(value);
                            }
                            _scrollUpdate();
                        });

                    _inOutRangeObserver = feather_tk::ValueObserver<OTIO_NS::TimeRange>::create(
                        file->getPlaybackModel()->observeInOutRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _inOutRange = value;
                            if (_timelineItem)
                            {
                                _timelineItem->setInOutRange(_inOutRange);
                            }
                        });
                }
                else
                {
                    _timeRange = OTIO_NS::TimeRange();
                    _timelineItem.reset();
                    _scrollWidget->setWidget(nullptr);
                    _currentTimeObserver.reset();
                }

                _setSizeUpdate();
                _setDrawUpdate();
            });
    }

    TimelineWidget::~TimelineWidget()
    {}

    std::shared_ptr<TimelineWidget> TimelineWidget::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget);
        out->_init(context, app, parent);
        return out;
    }

    void TimelineWidget::setViewZoom(double value)
    {
        const feather_tk::Box2I& g = getGeometry();
        setViewZoom(value, feather_tk::V2I(g.w() / 2, g.h() / 2));
    }

    void TimelineWidget::setViewZoom(double zoom, const feather_tk::V2I& focus)
    {
        _setViewZoom(
            zoom,
            _scale,
            focus,
            _scrollWidget->getScrollPos());
    }

    bool TimelineWidget::hasFrameView() const
    {
        return _frameView->get();
    }

    std::shared_ptr<feather_tk::IObservableValue<bool> > TimelineWidget::observeFrameView() const
    {
        return _frameView;
    }

    void TimelineWidget::setFrameView(bool value)
    {
        if (_frameView->setIfChanged(value))
        {
            if (value)
            {
                frameView();
            }
        }
    }

    void TimelineWidget::frameView()
    {
        feather_tk::V2I pos = _scrollWidget->getScrollPos();
        pos.x = 0;
        _scrollWidget->setScrollPos(pos);
        _scale = _getTimelineScale();
        if (_timelineItem)
        {
            _timelineItem->setScale(_scale);
        }
        _setSizeUpdate();
        _setDrawUpdate();
    }

    void TimelineWidget::setGeometry(const feather_tk::Box2I& value)
    {
        const feather_tk::Box2I viewportPrev = _scrollWidget->getViewport();
        IWidget::setGeometry(value);
        _scrollWidget->setGeometry(value);
        const bool changed = _scrollWidget->getViewport() != viewportPrev;
        if (_sizeInit)
        {
            _sizeInit = false;
            frameView();
        }
        else if (_viewState.has_value())
        {
            _scrollWidget->setScrollPos(_viewState->pos);
            setFrameView(_viewState->frameView);
            _viewState.reset();
        }
        else if (changed && _frameView->get())
        {
            frameView();
        }
        else if (_timelineItem &&
            _timelineItem->getSizeHint().w <
            _scrollWidget->getViewport().w())
        {
            setFrameView(true);
            frameView();
        }
    }

    void TimelineWidget::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_scrollWidget->getSizeHint());
    }

    void TimelineWidget::mouseMoveEvent(feather_tk::MouseMoveEvent& event)
    {
        IWidget::mouseMoveEvent(event);
        switch (_mouse.mode)
        {
        case MouseMode::Scroll:
        {
            const feather_tk::V2I d = event.pos - _getMousePressPos();
            _scrollWidget->setScrollPos(_mouse.scrollPos - d);
            setFrameView(false);
            break;
        }
        default: break;
        }
    }

    void TimelineWidget::mousePressEvent(feather_tk::MouseClickEvent& event)
    {
        IWidget::mousePressEvent(event);
        if (0 == event.button &&
            static_cast<int>(feather_tk::KeyModifier::Alt) == event.modifiers)
        {
            event.accept = true;
            takeKeyFocus();
            _mouse.mode = MouseMode::Scroll;
            _mouse.scrollPos = _scrollWidget->getScrollPos();
        }
        else
        {
            _mouse.mode = MouseMode::None;
        }
    }

    void TimelineWidget::mouseReleaseEvent(feather_tk::MouseClickEvent& event)
    {
        IWidget::mouseReleaseEvent(event);
    }

    void TimelineWidget::scrollEvent(feather_tk::ScrollEvent& event)
    {
        IWidget::scrollEvent(event);
        event.accept = true;
        if (event.value.y > 0)
        {
            const double zoom = _scale * 1.1;
            setViewZoom(zoom, event.pos);
        }
        else
        {
            const double zoom = _scale / 1.1;
            setViewZoom(zoom, event.pos);
        }
    }

    void TimelineWidget::keyPressEvent(feather_tk::KeyEvent& event)
    {
        if (0 == event.modifiers)
        {
            switch (event.key)
            {
            case feather_tk::Key::Equal:
                event.accept = true;
                setViewZoom(_scale * 2.0, event.pos);
                break;
            case feather_tk::Key::Minus:
                event.accept = true;
                setViewZoom(_scale / 2.0, event.pos);
                break;
            case feather_tk::Key::Backspace:
                event.accept = true;
                setFrameView(true);
                break;
            default: break;
            }
        }
    }

    void TimelineWidget::keyReleaseEvent(feather_tk::KeyEvent& event)
    {
        event.accept = true;
    }

    void TimelineWidget::_setViewZoom(
        double zoomNew,
        double zoomPrev,
        const feather_tk::V2I& focus,
        const feather_tk::V2I& scrollPos)
    {
        const int w = getGeometry().w();
        const double zoomMin = _getTimelineScale();
        const double zoomMax = _getTimelineScaleMax();
        const double zoomClamped = feather_tk::clamp(zoomNew, zoomMin, zoomMax);
        if (zoomClamped != _scale)
        {
            _scale = zoomClamped;
            if (_timelineItem)
            {
                _timelineItem->setScale(_scale);
            }
            const double s = zoomClamped / zoomPrev;
            const feather_tk::V2I scrollPosNew(
                (scrollPos.x + focus.x) * s - focus.x,
                scrollPos.y);
            _scrollWidget->setScrollPos(scrollPosNew, false);

            setFrameView(zoomNew <= zoomMin);
        }
    }

    double TimelineWidget::_getTimelineScale() const
    {
        double out = 1.0;
        const double duration = _timeRange.duration().rescaled_to(1.0).value();
        if (duration > 0.0)
        {
            const feather_tk::Box2I scrollViewport = _scrollWidget->getViewport();
            out = scrollViewport.w() / duration;
        }
        return out;
    }

    double TimelineWidget::_getTimelineScaleMax() const
    {
        double out = 1.0;
        const feather_tk::Box2I scrollViewport = _scrollWidget->getViewport();
        const double duration = _timeRange.duration().rescaled_to(1.0).value();
        if (duration < 1.0)
        {
            if (duration > 0.0)
            {
                out = scrollViewport.w() / duration;
            }
        }
        else
        {
            out = scrollViewport.w();
        }
        return out;
    }

    void TimelineWidget::_scrollUpdate()
    {
        if (_timelineItem &&
            MouseMode::None == _mouse.mode)
        {
            const int pos = _timelineItem->timeToPos(_currentTime);
            const feather_tk::Box2I vp = _scrollWidget->getViewport();
            const int margin = vp.w() * marginPercentage;
            if (pos < (vp.min.x + margin) || pos >(vp.max.x - margin))
            {
                const int offset = pos < (vp.min.x + margin) ? (vp.min.x + margin) : (vp.max.x - margin);
                const OTIO_NS::RationalTime t = _currentTime - _timeRange.start_time();
                feather_tk::V2I scrollPos = _scrollWidget->getScrollPos();
                const feather_tk::Box2I& g = getGeometry();
                scrollPos.x = g.min.x - offset + t.rescaled_to(1.0).value() * _scale;
                _scrollWidget->setScrollPos(scrollPos);
            }
        }
    }
}
