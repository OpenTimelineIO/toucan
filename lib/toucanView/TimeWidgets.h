// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/PlaybackModel.h>
#include <toucanView/TimeUnitsModel.h>

#include <feather-tk/ui/ButtonGroup.h>
#include <feather-tk/ui/IncButtons.h>
#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/LineEdit.h>
#include <feather-tk/ui/RowLayout.h>

#include <opentimelineio/anyVector.h>

namespace toucan
{
    //! Frame buttons.
    class FrameButtons : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~FrameButtons();

        //! Create a new widget.
        static std::shared_ptr<FrameButtons> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the callback.
        void setCallback(const std::function<void(TimeAction)>&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::function<void(TimeAction)> _callback;
        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::shared_ptr<feather_tk::ButtonGroup> _buttonGroup;
    };

    //! Playback buttons.
    class PlaybackButtons : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~PlaybackButtons();

        //! Create a new widget.
        static std::shared_ptr<PlaybackButtons> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the playback.
        void setPlayback(Playback);

        //! Set the callback.
        void setCallback(const std::function<void(Playback)>&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _playbackUpdate();

        Playback _playback = Playback::Stop;
        std::function<void(Playback)> _callback;
        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::shared_ptr<feather_tk::ButtonGroup> _buttonGroup;
    };

    //! Time edit.
    class TimeEdit : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<TimeUnitsModel>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimeEdit();

        //! Create a new widget.
        static std::shared_ptr<TimeEdit> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<TimeUnitsModel>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the time.
        void setTime(const OTIO_NS::RationalTime&);

        //! Set the time range.
        void setTimeRange(const OTIO_NS::TimeRange&);

        //! Set the callback.
        void setCallback(const std::function<void(const OTIO_NS::RationalTime&)>&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void keyPressEvent(feather_tk::KeyEvent&) override;
        void keyReleaseEvent(feather_tk::KeyEvent&) override;

    private:
        void _timeInc(int = 1);
        void _timeDec(int = 1);
        void _timeUpdate();

        OTIO_NS::RationalTime _time;
        OTIO_NS::TimeRange _timeRange;
        TimeUnits _timeUnits = TimeUnits::First;

        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::shared_ptr<feather_tk::LineEdit> _lineEdit;
        std::shared_ptr<feather_tk::IncButtons> _incButtons;
        std::function<void(const OTIO_NS::RationalTime&)> _callback;

        std::shared_ptr<feather_tk::ValueObserver<TimeUnits> > _timeUnitsObserver;
    };

    //! Time label.
    class TimeLabel : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<TimeUnitsModel>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimeLabel();

        //! Create a new widget.
        static std::shared_ptr<TimeLabel> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<TimeUnitsModel>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set the time.
        void setTime(const OTIO_NS::RationalTime&);

        //! Set the margin size role.
        void setMarginRole(feather_tk::SizeRole);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _timeUpdate();

        OTIO_NS::RationalTime _time;
        TimeUnits _timeUnits = TimeUnits::First;

        std::shared_ptr<feather_tk::Label> _label;

        std::shared_ptr<feather_tk::ValueObserver<TimeUnits> > _timeUnitsObserver;
    };
}

