// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimeWidgets.h"

#include <feather-tk/ui/ToolButton.h>

#include <sstream>

namespace toucan
{
    void FrameButtons::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<IWidget>& parent)
    {
        IWidget::_init(context, "toucan::FrameButtons", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        auto startButton = feather_tk::ToolButton::create(context, _layout);
        startButton->setIcon("FrameStart");
        startButton->setTooltip("Go to the start frame");

        auto prevButton = feather_tk::ToolButton::create(context, _layout);
        prevButton->setIcon("FramePrev");
        prevButton->setRepeatClick(true);
        prevButton->setTooltip("Go to the previous frame");

        auto nextButton = feather_tk::ToolButton::create(context, _layout);
        nextButton->setIcon("FrameNext");
        nextButton->setRepeatClick(true);
        nextButton->setTooltip("Go to the next frame");

        auto endButton = feather_tk::ToolButton::create(context, _layout);
        endButton->setIcon("FrameEnd");
        endButton->setTooltip("Go to the end frame");

        _buttonGroup = feather_tk::ButtonGroup::create(context, feather_tk::ButtonGroupType::Click);
        _buttonGroup->addButton(startButton);
        _buttonGroup->addButton(prevButton);
        _buttonGroup->addButton(nextButton);
        _buttonGroup->addButton(endButton);

        _buttonGroup->setClickedCallback(
            [this](int index)
            {
                if (_callback)
                {
                    _callback(static_cast<TimeAction>(index));
                }
            });
    }

    FrameButtons::~FrameButtons()
    {}

    std::shared_ptr<FrameButtons> FrameButtons::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::shared_ptr<FrameButtons>(new FrameButtons);
        out->_init(context, parent);
        return out;
    }

    void FrameButtons::setCallback(const std::function<void(TimeAction)>& value)
    {
        _callback = value;
    }

    void FrameButtons::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void FrameButtons::sizeHintEvent(const feather_tk::SizeHintEvent& value)
    {
        IWidget::sizeHintEvent(value);
        _setSizeHint(_layout->getSizeHint());
    }

    void PlaybackButtons::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<IWidget>& parent)
    {
        IWidget::_init(context, "toucan::PlaybackButtons", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        auto reverseButton = feather_tk::ToolButton::create(context, _layout);
        reverseButton->setIcon("PlaybackReverse");
        reverseButton->setTooltip("Reverse playback");

        auto stopButton = feather_tk::ToolButton::create(context, _layout);
        stopButton->setIcon("PlaybackStop");
        stopButton->setTooltip("Stop playback");

        auto forwardButton = feather_tk::ToolButton::create(context, _layout);
        forwardButton->setIcon("PlaybackForward");
        forwardButton->setTooltip("Forward playback");

        _buttonGroup = feather_tk::ButtonGroup::create(context, feather_tk::ButtonGroupType::Radio);
        _buttonGroup->addButton(stopButton);
        _buttonGroup->addButton(forwardButton);
        _buttonGroup->addButton(reverseButton);

        _playbackUpdate();

        _buttonGroup->setCheckedCallback(
            [this](int index, bool value)
            {
                if (value)
                {
                    if (_callback)
                    {
                        _callback(static_cast<Playback>(index));
                    }
                }
            });
    }

    PlaybackButtons::~PlaybackButtons()
    {}

    std::shared_ptr<PlaybackButtons> PlaybackButtons::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::shared_ptr<PlaybackButtons>(new PlaybackButtons);
        out->_init(context, parent);
        return out;
    }

    void PlaybackButtons::setPlayback(Playback value)
    {
        if (value == _playback)
            return;
        _playback = value;
        _playbackUpdate();
    }

    void PlaybackButtons::setCallback(const std::function<void(Playback)>& value)
    {
        _callback = value;
    }

    void PlaybackButtons::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void PlaybackButtons::sizeHintEvent(const feather_tk::SizeHintEvent& value)
    {
        IWidget::sizeHintEvent(value);
        _setSizeHint(_layout->getSizeHint());
    }

    void PlaybackButtons::_playbackUpdate()
    {
        _buttonGroup->setChecked(static_cast<int>(_playback), true);
    }

    void TimeEdit::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<TimeUnitsModel>& timeUnitsModel,
        const std::shared_ptr<IWidget>& parent)
    {
        IWidget::_init(context, "toucan::TimeEdit", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        _lineEdit = feather_tk::LineEdit::create(context, _layout);
        _lineEdit->setFontRole(feather_tk::FontRole::Mono);
        _lineEdit->setFormat("00:00:00:00");

        _incButtons = feather_tk::IncButtons::create(context, _layout);

        _timeUpdate();

        _lineEdit->setTextCallback(
            [this](const std::string& text)
            {
                if (_callback)
                {
                    _callback(fromString(text, _timeUnits, _time.rate()));
                }
            });

        _incButtons->setIncCallback(
            [this]
            {
                _timeInc();
            });
        _incButtons->setDecCallback(
            [this]
            {
                _timeDec();
            });

        _timeUnitsObserver = feather_tk::ValueObserver<TimeUnits>::create(
            timeUnitsModel->observeTimeUnits(),
            [this](TimeUnits value)
            {
                _timeUnits = value;
                _timeUpdate();
            });
    }

    TimeEdit::~TimeEdit()
    {}

    std::shared_ptr<TimeEdit> TimeEdit::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<TimeUnitsModel>& timeUnitsModel,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<TimeEdit>(new TimeEdit);
        out->_init(context, timeUnitsModel, parent);
        return out;
    }

    void TimeEdit::setTime(const OTIO_NS::RationalTime& value)
    {
        if (value.strictly_equal(_time))
            return;
        _time = value;
        _timeUpdate();
    }

    void TimeEdit::setTimeRange(const OTIO_NS::TimeRange& value)
    {
        if (value.start_time().strictly_equal(_timeRange.start_time()) &&
            value.duration().strictly_equal(_timeRange.duration()))
            return;
        _timeRange = value;
        _timeUpdate();
    }

    void TimeEdit::setCallback(const std::function<void(const OTIO_NS::RationalTime&)>& value)
    {
        _callback = value;
    }

    void TimeEdit::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void TimeEdit::sizeHintEvent(const feather_tk::SizeHintEvent& value)
    {
        IWidget::sizeHintEvent(value);
        _setSizeHint(_layout->getSizeHint());
    }

    void TimeEdit::keyPressEvent(feather_tk::KeyEvent& event)
    {
        if (_lineEdit->hasKeyFocus())
        {
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case feather_tk::Key::Up:
                case feather_tk::Key::Right:
                    event.accept = true;
                    _timeInc();
                    break;
                case feather_tk::Key::Down:
                case feather_tk::Key::Left:
                    event.accept = true;
                    _timeDec();
                    break;
                case feather_tk::Key::PageUp:
                    event.accept = true;
                    _timeInc(_time.rate());
                    break;
                case feather_tk::Key::PageDown:
                    event.accept = true;
                    _timeInc(-_time.rate());
                    break;
                default: break;
                }
            }
        }
        if (!event.accept)
        {
            IWidget::keyPressEvent(event);
        }
    }

    void TimeEdit::keyReleaseEvent(feather_tk::KeyEvent& event)
    {
        IWidget::keyReleaseEvent(event);
        event.accept = true;
    }

    void TimeEdit::_timeUpdate()
    {
        _lineEdit->setText(toString(_time, _timeUnits));
    }

    void TimeEdit::_timeInc(int value)
    {
        if (_callback)
        {
            const auto time = _time + OTIO_NS::RationalTime(value, _time.rate());
            _callback(time);
        }
    }

    void TimeEdit::_timeDec(int value)
    {
        if (_callback)
        {
            const auto time = _time - OTIO_NS::RationalTime(value, _time.rate());
            _callback(time);
        }
    }

    void TimeLabel::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<TimeUnitsModel>& timeUnitsModel,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IWidget::_init(context, "toucan::TimeLabel", parent);

        _label = feather_tk::Label::create(context, shared_from_this());
        _label->setFontRole(feather_tk::FontRole::Mono);
        _label->setMarginRole(feather_tk::SizeRole::MarginInside);

        _timeUpdate();

        _timeUnitsObserver = feather_tk::ValueObserver<TimeUnits>::create(
            timeUnitsModel->observeTimeUnits(),
            [this](TimeUnits value)
            {
                _timeUnits = value;
                _timeUpdate();
            });
    }

    TimeLabel::~TimeLabel()
    {}

    std::shared_ptr<TimeLabel> TimeLabel::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<TimeUnitsModel>& timeUnitsModel,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<TimeLabel>(new TimeLabel);
        out->_init(context, timeUnitsModel, parent);
        return out;
    }

    void TimeLabel::setTime(const OTIO_NS::RationalTime& value)
    {
        if (value.strictly_equal(_time))
            return;
        _time = value;
        _timeUpdate();
    }

    void TimeLabel::setMarginRole(feather_tk::SizeRole value)
    {
        _label->setMarginRole(value);
    }

    void TimeLabel::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _label->setGeometry(value);
    }

    void TimeLabel::sizeHintEvent(const feather_tk::SizeHintEvent& value)
    {
        IWidget::sizeHintEvent(value);
        _setSizeHint(_label->getSizeHint());
    }

    void TimeLabel::_timeUpdate()
    {
        _label->setText(toString(_time, _timeUnits));
    }
}
