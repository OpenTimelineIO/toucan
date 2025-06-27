// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ImageGraphTest.h"

#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>
#include <toucanRender/Util.h>

#include <sstream>

namespace toucan
{
    void imageGraphTest(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<ImageEffectHost>& host,
        const std::filesystem::path& path)
    {
        std::cout << "imageGraphTest" << std::endl;
        const std::vector<std::string> otioFiles =
        {
            "ColorSpace",
            "CompositeTracks",
            "Draw",
            "Filter",
            "Gap",
            "Generator",
            "LinearTimeWarp",
            "MultipleEffects",
            "Transform",
            "Transition",
            "Transition2",
            "TransitionWipe"
        };
        for (const auto& otioFile : otioFiles)
        {
            // Open the timeline.
            const std::filesystem::path timelinePath = path / (otioFile + ".otio");
            std::shared_ptr<TimelineWrapper> timelineWrapper;
            try
            {
                timelineWrapper = std::make_shared<TimelineWrapper>(timelinePath);
            }
            catch (const std::exception& e)
            {
                std::cout << "ERROR: " << e.what() << std::endl;
                continue;
            }

            // Get time values.
            const OTIO_NS::TimeRange& timeRange = timelineWrapper->getTimeRange();
            const OTIO_NS::RationalTime timeInc(1.0, timeRange.duration().rate());

            // Render the timeline frames.
            const auto graph = std::make_shared<ImageGraph>(context, path, timelineWrapper);
            for (OTIO_NS::RationalTime time = timeRange.start_time();
                time <= timeRange.end_time_inclusive();
                time += timeInc)
            {
                std::cout << "  " << otioFile << ": " << time.value() << "/" <<
                    timeRange.duration().value() << std::endl;
                if (auto op = graph->exec(host, time))
                {
                    // Execute the image operation graph.
                    const auto buf = op->exec();

                    // Save the image.
                    const std::string fileName = getSequenceFrame(
                        std::string(),
                        "imageGraphTest_" + otioFile + ".",
                        time.to_frames(),
                        6,
                        ".png");
                    buf.write(fileName);
                }
            }
        }
    }
}
