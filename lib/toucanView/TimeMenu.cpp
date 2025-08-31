// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimeMenu.h"

#include "App.h"
#include "FilesModel.h"
#include "PlaybackModel.h"

#include <feather-tk/ui/Action.h>

namespace toucan
{
    void TimeMenu::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::Menu::_init(context, parent);

        _actions["Time/FrameStart"] = ftk::Action::create(
            "Start Frame",
            "FrameStart",
            ftk::Key::Up,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FrameStart,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FrameStart"]);

        _actions["Time/FramePrev"] = ftk::Action::create(
            "Previous Frame",
            "FramePrev",
            ftk::Key::Left,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FramePrev,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FramePrev"]);

        _actions["Time/FramePrevX10"] = ftk::Action::create(
            "Previous Frame X10",
            ftk::Key::Left,
            static_cast<int>(ftk::KeyModifier::Shift),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FramePrevX10,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FramePrevX10"]);

        _actions["Time/FramePrevX100"] = ftk::Action::create(
            "Previous Frame X100",
            ftk::Key::Left,
            static_cast<int>(ftk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FramePrevX100,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FramePrevX100"]);

        _actions["Time/FrameNext"] = ftk::Action::create(
            "Next Frame",
            "FrameNext",
            ftk::Key::Right,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FrameNext,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FrameNext"]);

        _actions["Time/FrameNextX10"] = ftk::Action::create(
            "Next Frame X10",
            ftk::Key::Right,
            static_cast<int>(ftk::KeyModifier::Shift),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FrameNextX10,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FrameNextX10"]);

        _actions["Time/FrameNextX100"] = ftk::Action::create(
            "Next Frame X100",
            ftk::Key::Right,
            static_cast<int>(ftk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FrameNextX100,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FrameNextX100"]);

        _actions["Time/FrameEnd"] = ftk::Action::create(
            "End Frame",
            "FrameEnd",
            ftk::Key::Down,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::FrameEnd,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/FrameEnd"]);

        addDivider();

        _actions["Time/ClipNext"] = ftk::Action::create(
            "Next Clip",
            ftk::Key::Right,
            static_cast<int>(ftk::KeyModifier::Alt),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::ClipNext,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/ClipNext"]);

        _actions["Time/ClipPrev"] = ftk::Action::create(
            "Previous Clip",
            ftk::Key::Left,
            static_cast<int>(ftk::KeyModifier::Alt),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->timeAction(
                        TimeAction::ClipPrev,
                        _file->getTimeline());
                }
            });
        addAction(_actions["Time/ClipPrev"]);

        addDivider();

        _actions["Time/InPointSet"] = ftk::Action::create(
            "Set In Point",
            ftk::Key::I,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setInPoint(
                        _file->getPlaybackModel()->getCurrentTime());
                }
            });
        addAction(_actions["Time/InPointSet"]);

        _actions["Time/InPointReset"] = ftk::Action::create(
            "Reset In Point",
            ftk::Key::I,
            static_cast<int>(ftk::KeyModifier::Shift),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->resetInPoint();
                }
            });
        addAction(_actions["Time/InPointReset"]);

        _actions["Time/OutPointSet"] = ftk::Action::create(
            "Set Out Point",
            ftk::Key::O,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setOutPoint(
                        _file->getPlaybackModel()->getCurrentTime());
                }
            });
        addAction(_actions["Time/OutPointSet"]);

        _actions["Time/OutPointReset"] = ftk::Action::create(
            "Reset Out Point",
            ftk::Key::O,
            static_cast<int>(ftk::KeyModifier::Shift),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->resetOutPoint();
                }
            });
        addAction(_actions["Time/OutPointReset"]);

        _actions["Time/InOutPointReset"] = ftk::Action::create(
            "Reset In/Out Points",
            ftk::Key::P,
            static_cast<int>(ftk::KeyModifier::Shift),
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->resetInOutPoints();
                }
            });
        addAction(_actions["Time/InOutPointReset"]);

        _actions["Time/InOutPointSelection"] = ftk::Action::create(
            "Set In/Out Points To Selection",
            ftk::Key::P,
            static_cast<int>(ftk::KeyModifier::Shift) | static_cast<int>(ftk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    const auto selection = _file->getSelectionModel()->getSelection();
                    const OTIO_NS::TimeRange& timeRange = _file->getTimelineWrapper()->getTimeRange();
                    const auto timeRangeOpt = getTimeRange(
                        selection,
                        timeRange.start_time(),
                        timeRange.duration().rate());
                    if (timeRangeOpt.has_value())
                    {
                        _file->getPlaybackModel()->setInOutRange(timeRangeOpt.value());
                        _file->getPlaybackModel()->setCurrentTime(timeRangeOpt.value().start_time());
                    }
                    else
                    {
                        _file->getPlaybackModel()->resetInOutPoints();
                    }
                }
            });
        addAction(_actions["Time/InOutPointSelection"]);

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                if (_file)
                {
                    _timeRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
                        _file->getPlaybackModel()->observeTimeRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _timeRange = value;
                            _menuUpdate();
                        });
                    _inOutRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
                        _file->getPlaybackModel()->observeInOutRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _inOutRange = value;
                            _menuUpdate();
                        });
                    _selectionObserver = ftk::ListObserver<SelectionItem>::create(
                        _file->getSelectionModel()->observeSelection(),
                        [this](const std::vector<SelectionItem>& value)
                        {
                            _selection = !value.empty();
                            _menuUpdate();
                        });
                }
                else
                {
                    _timeRangeObserver.reset();
                    _inOutRangeObserver.reset();
                }
                _menuUpdate();
            });
    }

    TimeMenu::~TimeMenu()
    {}

    std::shared_ptr<TimeMenu> TimeMenu::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<TimeMenu>(new TimeMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& TimeMenu::getActions() const
    {
        return _actions;
    }

    void TimeMenu::_menuUpdate()
    {
        const bool file = _file.get();
        setEnabled(_actions["Time/FrameStart"], file);
        setEnabled(_actions["Time/FramePrev"], file);
        setEnabled(_actions["Time/FramePrevX10"], file);
        setEnabled(_actions["Time/FramePrevX100"], file);
        setEnabled(_actions["Time/FrameNext"], file);
        setEnabled(_actions["Time/FrameNextX10"], file);
        setEnabled(_actions["Time/FrameNextX100"], file);
        setEnabled(_actions["Time/FrameEnd"], file);
        setEnabled(_actions["Time/ClipPrev"], file);
        setEnabled(_actions["Time/ClipNext"], file);
        setEnabled(_actions["Time/InPointSet"], file);
        setEnabled(_actions["Time/OutPointSet"], file);
        setEnabled(_actions["Time/InPointReset"], file && _inOutRange != _timeRange);
        setEnabled(_actions["Time/OutPointReset"], file && _inOutRange != _timeRange);
        setEnabled(_actions["Time/InOutPointReset"], file && _inOutRange != _timeRange);
        setEnabled(_actions["Time/InOutPointSelection"], file && _selection);
    }
}
