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

    //! Timeline widget.
    class TimelineWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimelineWidget();

        //! Create a new widget.
        static std::shared_ptr<TimelineWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the view zoom.
        void setViewZoom(double);

        //! Set the view zoom with focus.
        void setViewZoom(double, const feather_tk::V2I& focus);

        //! Get whether frame view is enabled.
        bool hasFrameView() const;

        //! Observe whether frame view is enabled.
        std::shared_ptr<feather_tk::IObservableValue<bool> > observeFrameView() const;

        //! Set whether frame view is enabled.
        void setFrameView(bool);

        //! Frame the view.
        void frameView();

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void mouseMoveEvent(feather_tk::MouseMoveEvent&) override;
        void mousePressEvent(feather_tk::MouseClickEvent&) override;
        void mouseReleaseEvent(feather_tk::MouseClickEvent&) override;
        void scrollEvent(feather_tk::ScrollEvent&) override;
        void keyPressEvent(feather_tk::KeyEvent&) override;
        void keyReleaseEvent(feather_tk::KeyEvent&) override;

    private:
        void _setViewZoom(
            double zoomNew,
            double zoomPrev,
            const feather_tk::V2I& focus,
            const feather_tk::V2I& scrollPos);

        double _getTimelineScale() const;
        double _getTimelineScaleMax() const;

        void _scrollUpdate();

        std::shared_ptr<File> _file;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _currentTime;
        OTIO_NS::TimeRange _inOutRange;
        double _scale = 100.0;
        std::shared_ptr<feather_tk::ObservableValue<bool> > _frameView;
        bool _sizeInit = true;
        std::optional<TimelineViewState> _viewState;

        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<TimelineItem> _timelineItem;

        enum class MouseMode
        {
            None,
            Scroll
        };
        struct MouseData
        {
            MouseMode mode = MouseMode::None;
            feather_tk::V2I scrollPos;
            std::chrono::steady_clock::time_point wheelTimer;
        };
        MouseData _mouse;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
        std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::TimeRange> > _inOutRangeObserver;
    };
}
