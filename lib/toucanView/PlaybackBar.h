// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/TimeUnitsModel.h>
#include <toucanView/TimeWidgets.h>

#include <feather-tk/ui/ComboBox.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ToolButton.h>

namespace toucan
{
    class App;
    class File;

    //! Playback tool bar.
    class PlaybackBar : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~PlaybackBar();

        //! Create a new tool bar.
        static std::shared_ptr<PlaybackBar> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _timelineUpdate();
        void _timeRangeUpdate();
        void _currentTimeUpdate();
        void _playbackUpdate();

        std::shared_ptr<File> _file;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _currentTime;
        Playback _playback = Playback::Stop;

        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::shared_ptr<FrameButtons> _frameButtons;
        std::shared_ptr<PlaybackButtons> _playbackButtons;
        std::shared_ptr<TimeEdit> _timeEdit;
        std::shared_ptr<TimeLabel> _durationLabel;
        std::shared_ptr<feather_tk::ComboBox> _timeUnitsComboBox;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::TimeRange> > _timeRangeObserver;
        std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
        std::shared_ptr<feather_tk::ValueObserver<Playback> > _playbackObserver;
        std::shared_ptr<feather_tk::ValueObserver<TimeUnits> > _timeUnitsObserver;
    };
}

