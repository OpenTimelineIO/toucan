// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/PlaybackModel.h>
#include <toucanView/TimeUnitsModel.h>

#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/core/ObservableList.h>

#include <opentimelineio/timeline.h>

namespace toucan
{
    class App;
    class File;
    class TimelineItem;
    class ThumbnailGenerator;

    //! Timeline widget.
    class TimelineWidget : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimelineWidget();

        //! Create a new widget.
        static std::shared_ptr<TimelineWidget> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the view zoom.
        void setViewZoom(double);

        //! Set the view zoom with focus.
        void setViewZoom(double, const ftk::V2I& focus);

        //! Get whether frame view is enabled.
        bool hasFrameView() const;

        //! Observe whether frame view is enabled.
        std::shared_ptr<ftk::IObservableValue<bool> > observeFrameView() const;

        //! Set whether frame view is enabled.
        void setFrameView(bool);

        //! Frame the view.
        void frameView();

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void mouseMoveEvent(ftk::MouseMoveEvent&) override;
        void mousePressEvent(ftk::MouseClickEvent&) override;
        void mouseReleaseEvent(ftk::MouseClickEvent&) override;
        void scrollEvent(ftk::ScrollEvent&) override;
        void keyPressEvent(ftk::KeyEvent&) override;
        void keyReleaseEvent(ftk::KeyEvent&) override;

    private:
        void _setViewZoom(
            double zoomNew,
            double zoomPrev,
            const ftk::V2I& focus,
            const ftk::V2I& scrollPos);

        double _getTimelineScale() const;
        double _getTimelineScaleMax() const;

        void _scrollUpdate();

        std::shared_ptr<File> _file;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _currentTime;
        OTIO_NS::TimeRange _inOutRange;
        double _scale = 100.0;
        std::shared_ptr<ftk::ObservableValue<bool> > _frameView;
        bool _sizeInit = true;
        std::optional<TimelineViewState> _viewState;
        std::shared_ptr<ThumbnailGenerator> _thumbnailGenerator;

        std::shared_ptr<ftk::ScrollWidget> _scrollWidget;
        std::shared_ptr<TimelineItem> _timelineItem;

        enum class MouseMode
        {
            None,
            Scroll
        };
        struct MouseData
        {
            MouseMode mode = MouseMode::None;
            ftk::V2I scrollPos;
            std::chrono::steady_clock::time_point wheelTimer;
        };
        MouseData _mouse;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
        std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > _inOutRangeObserver;
    };
}
