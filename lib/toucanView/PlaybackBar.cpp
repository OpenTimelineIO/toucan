// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "PlaybackBar.h"

#include "App.h"
#include "FilesModel.h"

namespace toucan
{
    void PlaybackBar::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::PlaybackBar", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setMarginRole(feather_tk::SizeRole::MarginInside);
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);
        _layout->setVAlign(feather_tk::VAlign::Center);

        _playbackButtons = PlaybackButtons::create(context, _layout);

        _frameButtons = FrameButtons::create(context, _layout);

        _timeEdit = TimeEdit::create(context, app->getTimeUnitsModel(), _layout);
        _timeEdit->setTooltip("Current time");

        _durationLabel = TimeLabel::create(context, app->getTimeUnitsModel(), _layout);
        _durationLabel->setTooltip("Timeline duration");

        _timeUnitsComboBox = feather_tk::ComboBox::create(
            context,
            { "Timecode", "Frames", "Seconds" },
            _layout);
        _timeUnitsComboBox->setTooltip("Set the time units");

        _frameButtons->setCallback(
            [this](TimeAction value)
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        value,
                        _file->getTimeline());
                }
            });

        _playbackButtons->setCallback(
            [this](Playback value)
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setPlayback(value);
                }
            });

        _timeEdit->setCallback(
            [this](const OTIO_NS::RationalTime& value)
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setCurrentTime(value);
                }
            });

        auto appWeak = std::weak_ptr<App>(app);
        _timeUnitsComboBox->setIndexCallback(
            [appWeak](int value)
            {
                if (auto app = appWeak.lock())
                {
                    app->getTimeUnitsModel()->setTimeUnits(static_cast<TimeUnits>(value));
                }
            });

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                if (file)
                {
                    _timeRangeObserver = feather_tk::ValueObserver<OTIO_NS::TimeRange>::create(
                        file->getPlaybackModel()->observeTimeRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _timeRange = value;
                            _timeRangeUpdate();
                        });

                    _currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                        file->getPlaybackModel()->observeCurrentTime(),
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            _currentTime = value;
                            _currentTimeUpdate();
                        });

                    _playbackObserver = feather_tk::ValueObserver<Playback>::create(
                        file->getPlaybackModel()->observePlayback(),
                        [this](Playback value)
                        {
                            _playback = value;
                            _playbackUpdate();
                        });
                }
                else
                {
                    _timeRange = OTIO_NS::TimeRange();
                    _currentTime = OTIO_NS::RationalTime();
                    _playback = Playback::Stop;

                    _timeRangeUpdate();
                    _currentTimeUpdate();
                    _playbackUpdate();

                    _timeRangeObserver.reset();
                    _currentTimeObserver.reset();
                    _playbackObserver.reset();
                }

                _frameButtons->setEnabled(file.get());
                _playbackButtons->setEnabled(file.get());
                _timeEdit->setEnabled(file.get());
                _durationLabel->setEnabled(file.get());
            });

        _timeUnitsObserver = feather_tk::ValueObserver<TimeUnits>::create(
            app->getTimeUnitsModel()->observeTimeUnits(),
            [this](TimeUnits value)
            {
                _timeUnitsComboBox->setCurrentIndex(static_cast<int>(value));
            });
    }

        PlaybackBar::~PlaybackBar()
    {}

    std::shared_ptr<PlaybackBar> PlaybackBar::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<PlaybackBar>(new PlaybackBar);
        out->_init(context, app, parent);
        return out;
    }

    void PlaybackBar::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void PlaybackBar::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void PlaybackBar::_timelineUpdate()
    {}

    void PlaybackBar::_timeRangeUpdate()
    {
        _timeEdit->setTimeRange(_timeRange);

        _durationLabel->setTime(_timeRange.duration());
    }

    void PlaybackBar::_currentTimeUpdate()
    {
        _timeEdit->setTime(_currentTime);
    }

    void PlaybackBar::_playbackUpdate()
    {
        _playbackButtons->setPlayback(_playback);
    }
}
