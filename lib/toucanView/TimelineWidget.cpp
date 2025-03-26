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
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<IWidget>& parent)
    {
        IWidget::_init(context, "toucan::TimelineWidget", parent);

        _setMouseHoverEnabled(true);
        _setMousePressEnabled(
            true,
            0,
            static_cast<int>(dtk::KeyModifier::Alt));

        _frameView = dtk::ObservableValue<bool>::create(true);

        _scrollWidget = dtk::ScrollWidget::create(context, dtk::ScrollType::Both, shared_from_this());
        _scrollWidget->setScrollEventsEnabled(false);
        _scrollWidget->setBorder(false);

        auto appWeak = std::weak_ptr<App>(app);
        _fileObserver = dtk::ValueObserver<std::shared_ptr<File> >::create(
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
                    data.thumbnailCache = std::make_shared<dtk::LRUCache<std::string, std::shared_ptr<dtk::Image> > >();
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

                    _currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
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

                    _inOutRangeObserver = dtk::ValueObserver<OTIO_NS::TimeRange>::create(
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
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget);
        out->_init(context, app, parent);
        return out;
    }

    void TimelineWidget::setViewZoom(double value)
    {
        const dtk::Box2I& g = getGeometry();
        setViewZoom(value, dtk::V2I(g.w() / 2, g.h() / 2));
    }

    void TimelineWidget::setViewZoom(double zoom, const dtk::V2I& focus)
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

    std::shared_ptr<dtk::IObservableValue<bool> > TimelineWidget::observeFrameView() const
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
        dtk::V2I pos = _scrollWidget->getScrollPos();
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

    void TimelineWidget::setGeometry(const dtk::Box2I& value)
    {
        const dtk::Box2I viewportPrev = _scrollWidget->getViewport();
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

    void TimelineWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_scrollWidget->getSizeHint());
    }

    void TimelineWidget::mouseMoveEvent(dtk::MouseMoveEvent& event)
    {
        IWidget::mouseMoveEvent(event);
        switch (_mouse.mode)
        {
        case MouseMode::Scroll:
        {
            const dtk::V2I d = event.pos - _getMousePressPos();
            _scrollWidget->setScrollPos(_mouse.scrollPos - d);
            setFrameView(false);
            break;
        }
        default: break;
        }
    }

    void TimelineWidget::mousePressEvent(dtk::MouseClickEvent& event)
    {
        IWidget::mousePressEvent(event);
        if (0 == event.button &&
            static_cast<int>(dtk::KeyModifier::Alt) == event.modifiers)
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

    void TimelineWidget::mouseReleaseEvent(dtk::MouseClickEvent& event)
    {
        IWidget::mouseReleaseEvent(event);
    }

    void TimelineWidget::scrollEvent(dtk::ScrollEvent& event)
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

    void TimelineWidget::keyPressEvent(dtk::KeyEvent& event)
    {
        if (0 == event.modifiers)
        {
            switch (event.key)
            {
            case dtk::Key::Equal:
                event.accept = true;
                setViewZoom(_scale * 2.0, event.pos);
                break;
            case dtk::Key::Minus:
                event.accept = true;
                setViewZoom(_scale / 2.0, event.pos);
                break;
            case dtk::Key::Backspace:
                event.accept = true;
                setFrameView(true);
                break;
            default: break;
            }
        }
    }

    void TimelineWidget::keyReleaseEvent(dtk::KeyEvent& event)
    {
        event.accept = true;
    }

    void TimelineWidget::_setViewZoom(
        double zoomNew,
        double zoomPrev,
        const dtk::V2I& focus,
        const dtk::V2I& scrollPos)
    {
        const int w = getGeometry().w();
        const double zoomMin = _getTimelineScale();
        const double zoomMax = _getTimelineScaleMax();
        const double zoomClamped = dtk::clamp(zoomNew, zoomMin, zoomMax);
        if (zoomClamped != _scale)
        {
            _scale = zoomClamped;
            if (_timelineItem)
            {
                _timelineItem->setScale(_scale);
            }
            const double s = zoomClamped / zoomPrev;
            const dtk::V2I scrollPosNew(
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
            const dtk::Box2I scrollViewport = _scrollWidget->getViewport();
            out = scrollViewport.w() / duration;
        }
        return out;
    }

    double TimelineWidget::_getTimelineScaleMax() const
    {
        double out = 1.0;
        const dtk::Box2I scrollViewport = _scrollWidget->getViewport();
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
            const dtk::Box2I vp = _scrollWidget->getViewport();
            const int margin = vp.w() * marginPercentage;
            if (pos < (vp.min.x + margin) || pos >(vp.max.x - margin))
            {
                const int offset = pos < (vp.min.x + margin) ? (vp.min.x + margin) : (vp.max.x - margin);
                const OTIO_NS::RationalTime t = _currentTime - _timeRange.start_time();
                dtk::V2I scrollPos = _scrollWidget->getScrollPos();
                const dtk::Box2I& g = getGeometry();
                scrollPos.x = g.min.x - offset + t.rescaled_to(1.0).value() * _scale;
                _scrollWidget->setScrollPos(scrollPos);
            }
        }
    }
}
