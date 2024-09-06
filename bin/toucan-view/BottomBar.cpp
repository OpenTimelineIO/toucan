// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "BottomBar.h"

#include "App.h"

namespace toucan
{
    void BottomBar::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        dtk::IWidget::_init(context, "toucan::BottomBar", parent);

        _layout = dtk::HorizontalLayout::create(context, shared_from_this());
        _layout->setMarginRole(dtk::SizeRole::MarginInside);
        _layout->setSpacingRole(dtk::SizeRole::SpacingTool);

        _playbackButtons = PlaybackButtons::create(context, _layout);

        _frameButtons = FrameButtons::create(context, _layout);

        _timeEdit = TimeEdit::create(context, app->getTimeUnitsModel(), _layout);
        _timeEdit->setTooltip("Current time");

        _slider = dtk::IntSlider::create(context, nullptr, _layout);

        _durationLabel = TimeLabel::create(context, app->getTimeUnitsModel(), _layout);
        _durationLabel->setTooltip("Timeline duration");

        _timeUnitsComboBox = dtk::ComboBox::create(context, _layout);
        _timeUnitsComboBox->setItems({ "Timecode", "Frames", "Seconds" });
        _timeUnitsComboBox->setTooltip("Time units");

        _frameButtons->setCallback(
            [this](FrameAction value)
            {
                if (_document)
                {
                    _document->getPlaybackModel()->frameAction(value);
                }
            });

        _playbackButtons->setCallback(
            [this](Playback value)
            {
                if (_document)
                {
                    _document->getPlaybackModel()->setPlayback(value);
                }
            });

        _timeEdit->setCallback(
            [this](const OTIO_NS::RationalTime& value)
            {
                if (_document)
                {
                    _document->getPlaybackModel()->setCurrentTime(value);
                }
            });

        _slider->setCallback(
            [this](double value)
            {
                if (_document)
                {
                    _document->getPlaybackModel()->setCurrentTime(OTIO_NS::RationalTime(
                        value,
                        _timeRange.duration().rate()));
                }
            });

        std::weak_ptr<App> appWeak(app);
        _timeUnitsComboBox->setIndexCallback(
            [appWeak](int index)
            {
                if (auto app = appWeak.lock())
                {
                    app->getTimeUnitsModel()->setTimeUnits(static_cast<TimeUnits>(index));
                }
            });

        _documentObserver = dtk::ValueObserver<std::shared_ptr<Document> >::create(
            app->getDocumentsModel()->observeCurrent(),
            [this](const std::shared_ptr<Document>& document)
            {
                _document = document;
                if (document)
                {
                    _timeRangeObserver = dtk::ValueObserver<OTIO_NS::TimeRange>::create(
                        document->getPlaybackModel()->observeTimeRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _timeRange = value;
                            _timeRangeUpdate();
                        });

                    _currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                        document->getPlaybackModel()->observeCurrentTime(),
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            _currentTime = value;
                            _currentTimeUpdate();
                        });

                    _playbackObserver = dtk::ValueObserver<Playback>::create(
                        document->getPlaybackModel()->observePlayback(),
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

                _frameButtons->setEnabled(document.get());
                _playbackButtons->setEnabled(document.get());
                _timeEdit->setEnabled(document.get());
                _slider->setEnabled(document.get());
                _durationLabel->setEnabled(document.get());
            });

        _timeUnitsObserver = dtk::ValueObserver<TimeUnits>::create(
            app->getTimeUnitsModel()->observeTimeUnits(),
            [this](TimeUnits value)
            {
                _timeUnitsComboBox->setCurrentIndex(static_cast<int>(value));
            });
    }

    BottomBar::~BottomBar()
    {}

    std::shared_ptr<BottomBar> BottomBar::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        auto out = std::shared_ptr<BottomBar>(new BottomBar);
        out->_init(context, app, parent);
        return out;
    }

    void BottomBar::setGeometry(const dtk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void BottomBar::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void BottomBar::_timelineUpdate()
    {}

    void BottomBar::_timeRangeUpdate()
    {
        _timeEdit->setTimeRange(_timeRange);

        _slider->setRange(dtk::RangeI(
            _timeRange.start_time().value(),
            _timeRange.end_time_inclusive().value()));

        _durationLabel->setTime(_timeRange.duration());
    }

    void BottomBar::_currentTimeUpdate()
    {
        _timeEdit->setTime(_currentTime);

        _slider->setValue(_currentTime.value());
    }

    void BottomBar::_playbackUpdate()
    {
        _playbackButtons->setPlayback(_playback);
    }
}
