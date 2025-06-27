// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/IWidget.h>

#include <opentimelineio/version.h>

namespace toucan
{
    //! Base class for widgets in a time layout.
    class ITimeWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::TimeRange&,
            const std::string& objectName,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ITimeWidget() = 0;

        //! Get the time range.
        const OTIO_NS::TimeRange& getTimeRange();

        //! Get the scale.
        double getScale() const;

        //! Set the scale.
        virtual void setScale(double);

        //! Get the minimum width.
        int getMinWidth() const;

        //! Convert a position to a time.
        OTIO_NS::RationalTime posToTime(double) const;

        //! Convert a time to a position.
        int timeToPos(const OTIO_NS::RationalTime&) const;

    protected:
        OTIO_NS::TimeRange _timeRange;
        double _scale = 100.0;
        int _minWidth = 0;
    };

    //! Time layout.
    class TimeLayout : public ITimeWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimeLayout();

        //! Create a new layout.
        static std::shared_ptr<TimeLayout> create(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
    };

    //! Time stack layout.
    class TimeStackLayout : public ITimeWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimeStackLayout();

        //! Create a new layout.
        static std::shared_ptr<TimeStackLayout> create(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int spacing = 0;
        };
        SizeData _size;
    };
}

