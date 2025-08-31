// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/Settings.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/ObservableValue.h>

#include <opentimelineio/version.h>

namespace toucan
{
    //! Time units.
    enum class TimeUnits
    {
        Timecode,
        Frames,
        Seconds,

        Count,
        First = Timecode
    };
    FTK_ENUM(TimeUnits);

    //! Convert a time to a string.
    std::string toString(const OTIO_NS::RationalTime&, TimeUnits);

    //! Convert a string to a time.
    OTIO_NS::RationalTime fromString(const std::string&, TimeUnits, double rate);

    //! Convert a range to a string.
    std::string toString(const OTIO_NS::TimeRange&, TimeUnits);

    //! Time units model. 
    class TimeUnitsModel : public std::enable_shared_from_this<TimeUnitsModel>
    {
    public:
        TimeUnitsModel(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<ftk::Settings>&);

        virtual ~TimeUnitsModel();

        //! Get the time units.
        TimeUnits getTimeUnits() const;

        //! Observe the time units.
        std::shared_ptr<ftk::IObservableValue<TimeUnits> > observeTimeUnits() const;

        //! Set the time units.
        void setTimeUnits(TimeUnits);

    private:
        std::shared_ptr<ftk::Settings> _settings;
        std::shared_ptr<ftk::ObservableValue<TimeUnits> > _timeUnits;
    };
}
