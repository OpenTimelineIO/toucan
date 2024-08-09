// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "TimelineTraverseTest.h"

#include <toucan/TimelineTraverse.h>

#include <iomanip>
#include <sstream>

namespace toucan
{
    void timelineTraverseTest(const std::filesystem::path& path)
    {
        std::cout << "timelineTraverseTest" << std::endl;

        const std::filesystem::path timelinePath = path / "SimpleOver.otio";
        if (auto timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(
            dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(timelinePath.string()))))
        {
            const OTIO_NS::RationalTime startTime = timeline->global_start_time().has_value() ?
                timeline->global_start_time().value() :
                OTIO_NS::RationalTime(0.0, timeline->duration().rate());
            const OTIO_NS::TimeRange timeRange(startTime, timeline->duration());
            const OTIO_NS::RationalTime timeInc(1.0, timeline->duration().rate());

            auto traverse = std::make_shared<TimelineTraverse>(path, timeline);
            for (OTIO_NS::RationalTime time = startTime;
                time <= timeRange.end_time_inclusive();
                time += timeInc)
            {
                std::cout << "  frame: " << time.value() << std::endl;
                if (auto op = traverse->exec(time))
                {
                    auto buf = op->exec();
                    std::stringstream ss;
                    ss << "TimelineTraverseTest." <<
                        std::setw(6) << std::setfill('0') << time.to_frames() <<
                        ".png";
                    buf.write(ss.str());
                }
            }
        }
    }
}