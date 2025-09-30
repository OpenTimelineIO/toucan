// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <opentimelineio/timeline.h>

#include <ftk/Core/ObservableValue.h>
#include <ftk/Core/Timer.h>
#include <ftk/Core/Vector.h>

#include <optional>

namespace toucan
{
    //! Current time behavior.
    enum CurrentTime
    {
        Free,
        Clamp,
        Loop
    };

    //! Time actions.
    enum TimeAction
    {
        FrameStart,
        FramePrev,
        FramePrevX10,
        FramePrevX100,
        FrameNext,
        FrameNextX10,
        FrameNextX100,
        FrameEnd,
        ClipNext,
        ClipPrev
    };

    //! Playback actions.
    enum class Playback
    {
        Stop,
        Forward,
        Reverse
    };

    //! Timeline view state.
    struct TimelineViewState
    {
        ftk::V2I pos;
        double scale = 1.0;
        bool frameView = true;
    };

    //! Playback model.
    class PlaybackModel : public std::enable_shared_from_this<PlaybackModel>
    {
    public:
        PlaybackModel(const std::shared_ptr<ftk::Context>&);

        virtual ~PlaybackModel();

        //! Get the time range.
        const OTIO_NS::TimeRange& getTimeRange() const;

        //! Observe the time range.
        std::shared_ptr<ftk::IObservableValue<OTIO_NS::TimeRange> > observeTimeRange() const;

        //! Set the time range.
        void setTimeRange(const OTIO_NS::TimeRange&);

        //! Get the current time.
        const OTIO_NS::RationalTime& getCurrentTime() const;

        //! Observe the current time.
        std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > observeCurrentTime() const;

        //! Set the current time.
        void setCurrentTime(
            const OTIO_NS::RationalTime&,
            CurrentTime = CurrentTime::Clamp);

        //! Time actions.
        void timeAction(
            TimeAction,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&);

        //! Get the in/out range.
        const OTIO_NS::TimeRange& getInOutRange() const;

        //! Observe the in/out range.
        std::shared_ptr<ftk::IObservableValue<OTIO_NS::TimeRange> > observeInOutRange() const;

        //! Set the in/out range.
        void setInOutRange(const OTIO_NS::TimeRange&);

        //! Set the in point.
        void setInPoint(const OTIO_NS::RationalTime&);

        //! Reset the in point.
        void resetInPoint();

        //! Set the out point.
        void setOutPoint(const OTIO_NS::RationalTime&);

        //! Reset the in point.
        void resetOutPoint();

        //! Reset the in/out points.
        void resetInOutPoints();

        //! Get the playback.
        Playback getPlayback() const;

        //! Observe the playback.
        std::shared_ptr<ftk::IObservableValue<Playback> > observePlayback() const;

        //! Set the playback.
        void setPlayback(Playback);

        //! Toggle the playback.
        void togglePlayback();

        //! Get the timeline view state.
        const std::optional<TimelineViewState>& getViewState() const;

        //! Set the timeline view state.
        void setViewState(const std::optional<TimelineViewState>&);

    private:
        void _timeUpdate();

        std::shared_ptr<ftk::ObservableValue<OTIO_NS::TimeRange> > _timeRange;
        std::shared_ptr<ftk::ObservableValue<OTIO_NS::RationalTime> > _currentTime;
        std::shared_ptr<ftk::ObservableValue<OTIO_NS::TimeRange> > _inOutRange;
        std::shared_ptr<ftk::ObservableValue<Playback> > _playback;
        Playback _playbackPrev = Playback::Forward;
        std::optional<TimelineViewState> _viewState;
        std::shared_ptr<ftk::Timer> _timer;
    };
}
