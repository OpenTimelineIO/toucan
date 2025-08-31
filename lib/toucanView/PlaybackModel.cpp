// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "PlaybackModel.h"

#include <toucanRender/TimelineAlgo.h>

#include <opentimelineio/clip.h>

namespace toucan
{
    PlaybackModel::PlaybackModel(const std::shared_ptr<ftk::Context>& context)
    {
        _timeRange = ftk::ObservableValue<OTIO_NS::TimeRange>::create();
        _currentTime = ftk::ObservableValue<OTIO_NS::RationalTime>::create(OTIO_NS::RationalTime(-1.0, -1.0));
        _inOutRange = ftk::ObservableValue<OTIO_NS::TimeRange>::create();
        _playback = ftk::ObservableValue<Playback>::create(Playback::Stop);
        _timer = ftk::Timer::create(context);
        _timer->setRepeating(true);
    }

    PlaybackModel::~PlaybackModel()
    {}

    const OTIO_NS::TimeRange& PlaybackModel::getTimeRange() const
    {
        return _timeRange->get();
    }

    std::shared_ptr<ftk::IObservableValue<OTIO_NS::TimeRange> > PlaybackModel::observeTimeRange() const
    {
        return _timeRange;
    }

    void PlaybackModel::setTimeRange(const OTIO_NS::TimeRange& value)
    {
        if (_timeRange->setIfChanged(value))
        {
            _inOutRange->setIfChanged(value);
            _currentTime->setIfChanged(value.start_time());
        }
    }

    const OTIO_NS::RationalTime& PlaybackModel::getCurrentTime() const
    {
        return _currentTime->get();
    }

    std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > PlaybackModel::observeCurrentTime() const
    {
        return _currentTime;
    }

    void PlaybackModel::setCurrentTime(
        const OTIO_NS::RationalTime& value,
        CurrentTime behavior)
    {
        const OTIO_NS::TimeRange& timeRange = _inOutRange->get();
        OTIO_NS::RationalTime time = value.rescaled_to(timeRange.duration()).round();
        if (!time.is_invalid_time())
        {
            switch (behavior)
            {
            case CurrentTime::Clamp:
                if (time > timeRange.end_time_inclusive())
                {
                    time = timeRange.end_time_inclusive();
                }
                else if (time < timeRange.start_time())
                {
                    time = timeRange.start_time();
                }
                break;
            case CurrentTime::Loop:
                if (time > timeRange.end_time_inclusive())
                {
                    time = timeRange.start_time();
                }
                else if (time < timeRange.start_time())
                {
                    time = timeRange.end_time_inclusive();
                }
                break;
            default: break;
            }
        }
        _currentTime->setIfChanged(time);
    }

    void PlaybackModel::timeAction(
        TimeAction value,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline)
    {
        const OTIO_NS::TimeRange& timeRange = _timeRange->get();
        const OTIO_NS::RationalTime& currentTime = _currentTime->get();
        switch (value)
        {
        case TimeAction::FrameStart:
            setCurrentTime(timeRange.start_time());
            break;
        case TimeAction::FramePrev:
            setCurrentTime(currentTime - OTIO_NS::RationalTime(1.0, currentTime.rate()));
            break;
        case TimeAction::FramePrevX10:
            setCurrentTime(currentTime - OTIO_NS::RationalTime(10.0, currentTime.rate()));
            break;
        case TimeAction::FramePrevX100:
            setCurrentTime(currentTime - OTIO_NS::RationalTime(100.0, currentTime.rate()));
            break;
        case TimeAction::FrameNext:
            setCurrentTime(currentTime + OTIO_NS::RationalTime(1.0, currentTime.rate()));
            break;
        case TimeAction::FrameNextX10:
            setCurrentTime(currentTime + OTIO_NS::RationalTime(10.0, currentTime.rate()));
            break;
        case TimeAction::FrameNextX100:
            setCurrentTime(currentTime + OTIO_NS::RationalTime(100.0, currentTime.rate()));
            break;
        case TimeAction::FrameEnd:
            setCurrentTime(timeRange.end_time_inclusive());
            break;
        case TimeAction::ClipNext:
        {
            std::optional<OTIO_NS::RationalTime> min;
            std::optional<OTIO_NS::RationalTime> diff;
            std::optional<OTIO_NS::RationalTime> t;
            const auto clips = getVideoClips(timeline);
            auto i = clips.begin();
            if (i != clips.end())
            {
                const auto clipRangeOpt = (*i)->trimmed_range_in_parent();
                if (clipRangeOpt.has_value())
                {
                    const OTIO_NS::RationalTime start =
                        clipRangeOpt.value().start_time() +
                        timeRange.start_time();
                    min = start;
                    if (start > currentTime)
                    {
                        diff = currentTime - start;
                        diff = OTIO_NS::RationalTime(std::abs(diff.value().value()), diff.value().rate());
                        t = start;
                    }
                }
                ++i;
            }
            for (; i != clips.end(); ++i)
            {
                const auto clipRangeOpt = (*i)->trimmed_range_in_parent();
                if (clipRangeOpt.has_value())
                {
                    const OTIO_NS::RationalTime start =
                        clipRangeOpt.value().start_time() +
                        timeRange.start_time();
                    min = min.has_value() ? std::min(min.value(), start) : start;
                    if (start > currentTime)
                    {
                        OTIO_NS::RationalTime clipDiff = currentTime - start;
                        clipDiff = OTIO_NS::RationalTime(std::abs(clipDiff.value()), clipDiff.rate());
                        if (!diff.has_value() || clipDiff < diff)
                        {
                            diff = clipDiff;
                            t = start;
                        }
                    }
                }
            }
            if (t.has_value())
            {
                setCurrentTime(t.value(), CurrentTime::Free);
            }
            else if (min.has_value())
            {
                setCurrentTime(min.value(), CurrentTime::Free);
            }
            break;
        }
        case TimeAction::ClipPrev:
        {
            std::optional<OTIO_NS::RationalTime> max;
            std::optional<OTIO_NS::RationalTime> diff;
            std::optional<OTIO_NS::RationalTime> t;
            const auto clips = getVideoClips(timeline);
            auto i = clips.begin();
            if (i != clips.end())
            {
                const auto clipRangeOpt = (*i)->trimmed_range_in_parent();
                if (clipRangeOpt.has_value())
                {
                    const OTIO_NS::RationalTime start =
                        clipRangeOpt.value().start_time() +
                        timeRange.start_time();
                    max = start;
                    if (start < currentTime)
                    {
                        diff = currentTime - start;
                        diff = OTIO_NS::RationalTime(std::abs(diff.value().value()), diff.value().rate());
                        t = start;
                    }
                }
                ++i;
            }
            for (; i != clips.end(); ++i)
            {
                const auto clipRangeOpt = (*i)->trimmed_range_in_parent();
                if (clipRangeOpt.has_value())
                {
                    const OTIO_NS::RationalTime start =
                        clipRangeOpt.value().start_time() +
                        timeRange.start_time();
                    max = max.has_value() ? std::max(max.value(), start) : start;
                    if (start < currentTime)
                    {
                        OTIO_NS::RationalTime clipDiff = currentTime - start;
                        clipDiff = OTIO_NS::RationalTime(std::abs(clipDiff.value()), clipDiff.rate());
                        if (!diff.has_value() || clipDiff < diff)
                        {
                            diff = clipDiff;
                            t = start;
                        }
                    }
                }
            }
            if (t.has_value())
            {
                setCurrentTime(t.value(), CurrentTime::Free);
            }
            else if (max.has_value())
            {
                setCurrentTime(max.value(), CurrentTime::Free);
            }
            break;
        }
        default: break;
        }
    }

    const OTIO_NS::TimeRange& PlaybackModel::getInOutRange() const
    {
        return _inOutRange->get();
    }

    std::shared_ptr<ftk::IObservableValue<OTIO_NS::TimeRange> > PlaybackModel::observeInOutRange() const
    {
        return _inOutRange;
    }

    void PlaybackModel::setInOutRange(const OTIO_NS::TimeRange& value)
    {
        const OTIO_NS::TimeRange timeRange = _timeRange->get();
        OTIO_NS::TimeRange tmp(
            value.start_time().rescaled_to(timeRange.duration()).round(),
            value.duration().rescaled_to(timeRange.duration()).round());
        //! \bug OTIO_NS::TimeRange::clamped() seems to be off by one?
        //const OTIO_NS::TimeRange clamped = value.clamped(_timeRange->get());
        OTIO_NS::TimeRange clamped = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
            std::max(tmp.start_time(), timeRange.start_time()),
            std::min(tmp.end_time_inclusive(), timeRange.end_time_inclusive()));
        _inOutRange->setIfChanged(clamped);
        if (_currentTime->get() < clamped.start_time())
        {
            setCurrentTime(clamped.start_time());
        }
        else if (_currentTime->get() > clamped.end_time_inclusive())
        {
            setCurrentTime(clamped.end_time_inclusive());
        }
    }

    void PlaybackModel::setInPoint(const OTIO_NS::RationalTime& value)
    {
        setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
            value,
            _inOutRange->get().end_time_inclusive()));
    }

    void PlaybackModel::resetInPoint()
    {
        setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
            _timeRange->get().start_time(),
            _inOutRange->get().end_time_inclusive()));
    }

    void PlaybackModel::setOutPoint(const OTIO_NS::RationalTime& value)
    {
        setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
            _inOutRange->get().start_time(),
            value));
    }

    void PlaybackModel::resetOutPoint()
    {
        setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
            _inOutRange->get().start_time(),
            _timeRange->get().end_time_inclusive()));
    }

    void PlaybackModel::resetInOutPoints()
    {
        setInOutRange(_timeRange->get());
    }

    Playback PlaybackModel::getPlayback() const
    {
        return _playback->get();
    }

    std::shared_ptr<ftk::IObservableValue<Playback> > PlaybackModel::observePlayback() const
    {
        return _playback;
    }

    void PlaybackModel::setPlayback(Playback value)
    {
        const Playback prev = _playback->get();
        if (_playback->setIfChanged(value))
        {
            switch (value)
            {
            case Playback::Stop:
                _timer->stop();
                _playbackPrev = prev;
                break;
            case Playback::Forward:
            case Playback::Reverse:
                setCurrentTime(_currentTime->get());
                _timer->start(
                    std::chrono::microseconds(static_cast<int>(1000 / _currentTime->get().rate())),
                    [this]
                    {
                        _timeUpdate();
                    });
                break;
            default: break;
            }
        }
    }

    void PlaybackModel::togglePlayback()
    {
        setPlayback(
            Playback::Stop == _playback->get() ?
            _playbackPrev :
            Playback::Stop);
    }

    const std::optional<TimelineViewState>& PlaybackModel::getViewState() const
    {
        return _viewState;
    }

    void PlaybackModel::setViewState(const std::optional<TimelineViewState>& value)
    {
        _viewState = value;
    }

    void PlaybackModel::_timeUpdate()
    {
        switch (_playback->get())
        {
        case Playback::Forward:
            setCurrentTime(
                _currentTime->get() + OTIO_NS::RationalTime(1.0, _currentTime->get().rate()),
                CurrentTime::Loop);
            break;
        case Playback::Reverse:
            setCurrentTime(
                _currentTime->get() - OTIO_NS::RationalTime(1.0, _currentTime->get().rate()),
                CurrentTime::Loop);
            break;
        default: break;
        }
    }
}
