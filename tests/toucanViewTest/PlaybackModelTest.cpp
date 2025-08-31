// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FilesModelTest.h"

#include <toucanView/PlaybackModel.h>

#include <opentimelineio/clip.h>

#include <cassert>
#include <iostream>

namespace toucan
{
    namespace
    {
        class Test
        {
        public:
            Test(const std::shared_ptr<ftk::Context>& context)
            {
                model = std::make_shared<PlaybackModel>(context);

                timeRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
                    model->observeTimeRange(),
                    [this](const OTIO_NS::TimeRange& value)
                    {
                        timeRange = value;
                    });

                currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
                    model->observeCurrentTime(),
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        currentTime = value;
                    });

                inOutRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
                    model->observeInOutRange(),
                    [this](const OTIO_NS::TimeRange& value)
                    {
                        inOutRange = value;
                    });

                playbackObserver = ftk::ValueObserver<Playback>::create(
                    model->observePlayback(),
                    [this](Playback value)
                    {
                        playback = value;
                    });
            }

            std::shared_ptr<PlaybackModel> model;
            OTIO_NS::TimeRange timeRange;
            OTIO_NS::RationalTime currentTime;
            OTIO_NS::TimeRange inOutRange;
            Playback playback = Playback::Stop;

            std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > timeRangeObserver;
            std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > inOutRangeObserver;
            std::shared_ptr<ftk::ValueObserver<Playback> > playbackObserver;
        };
    }

    void playbackModelTest(
        const std::shared_ptr<ftk::Context>& context,
        const std::filesystem::path& path)
    {
        std::cout << "playbackModelTest" << std::endl;
        {
            Test test(context);
            const OTIO_NS::TimeRange timeRange(
                OTIO_NS::RationalTime(0.0, 24.0),
                OTIO_NS::RationalTime(48.0, 24.0));
            test.model->setTimeRange(timeRange);
            assert(timeRange == test.timeRange);
            assert(timeRange == test.inOutRange);
            assert(timeRange.start_time() == test.currentTime);
        }
        {
            Test test(context);
            test.model->setTimeRange(OTIO_NS::TimeRange(
                OTIO_NS::RationalTime(0.0, 24.0),
                OTIO_NS::RationalTime(48.0, 24.0)));
            test.model->setCurrentTime(OTIO_NS::RationalTime(24.0, 24.0));
            assert(OTIO_NS::RationalTime(24.0, 24.0) == test.currentTime);
            test.model->setCurrentTime(OTIO_NS::RationalTime(48.0, 24.0));
            assert(OTIO_NS::RationalTime(47.0, 24.0) == test.currentTime);
            test.model->setCurrentTime(OTIO_NS::RationalTime(48.0, 24.0), CurrentTime::Loop);
            assert(OTIO_NS::RationalTime(0.0, 24.0) == test.currentTime);

            test.model->timeAction(TimeAction::FrameEnd, nullptr);
            assert(OTIO_NS::RationalTime(47.0, 24.0) == test.currentTime);
            test.model->timeAction(TimeAction::FrameStart, nullptr);
            assert(OTIO_NS::RationalTime(0.0, 24.0) == test.currentTime);

            test.model->timeAction(TimeAction::FrameNext, nullptr);
            assert(OTIO_NS::RationalTime(1.0, 24.0) == test.currentTime);
            test.model->timeAction(TimeAction::FrameNextX10, nullptr);
            assert(OTIO_NS::RationalTime(11.0, 24.0) == test.currentTime);
            test.model->timeAction(TimeAction::FrameNextX100, nullptr);
            assert(OTIO_NS::RationalTime(47.0, 24.0) == test.currentTime);

            test.model->timeAction(TimeAction::FramePrev, nullptr);
            assert(OTIO_NS::RationalTime(46.0, 24.0) == test.currentTime);
            test.model->timeAction(TimeAction::FramePrevX10, nullptr);
            assert(OTIO_NS::RationalTime(36.0, 24.0) == test.currentTime);
            test.model->timeAction(TimeAction::FramePrevX100, nullptr);
            assert(OTIO_NS::RationalTime(0.0, 24.0) == test.currentTime);
        }
        {
            Test test(context);
            test.model->setTimeRange(OTIO_NS::TimeRange(
                OTIO_NS::RationalTime(0.0, 24.0),
                OTIO_NS::RationalTime(48.0, 24.0)));
            const OTIO_NS::TimeRange inOutRange(
                OTIO_NS::RationalTime(10.0, 24.0),
                OTIO_NS::RationalTime(10.0, 24.0));
            test.model->setInOutRange(inOutRange);
            assert(inOutRange == test.inOutRange);
            assert(inOutRange.start_time() == test.currentTime);
            test.model->timeAction(TimeAction::FrameEnd, nullptr);
            assert(OTIO_NS::RationalTime(19.0, 24.0) == test.currentTime);
            test.model->timeAction(TimeAction::FrameStart, nullptr);
            assert(OTIO_NS::RationalTime(10.0, 24.0) == test.currentTime);
        }
        {
            const std::filesystem::path timelinePath = path / "Gap.otio";
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> timeline(
                dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(timelinePath.string())));
            const auto clips = timeline->find_clips();

            Test test(context);
            test.model->setTimeRange(OTIO_NS::TimeRange(
                OTIO_NS::RationalTime(0.0, timeline->duration().rate()),
                timeline->duration()));
            test.model->timeAction(TimeAction::ClipNext, timeline);
            assert(clips[1]->trimmed_range_in_parent().value().start_time() == test.currentTime);
            test.model->timeAction(TimeAction::ClipPrev, timeline);
            assert(clips[0]->trimmed_range_in_parent().value().start_time() == test.currentTime);
        }
        {
            Test test(context);
            test.model->setPlayback(Playback::Forward);
            assert(Playback::Forward == test.playback);
            test.model->togglePlayback();
            assert(Playback::Stop == test.playback);
            test.model->togglePlayback();
            assert(Playback::Forward == test.playback);

            test.model->setPlayback(Playback::Reverse);
            assert(Playback::Reverse == test.playback);
            test.model->togglePlayback();
            assert(Playback::Stop == test.playback);
            test.model->togglePlayback();
            assert(Playback::Reverse == test.playback);
        }
    }
}
