// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ImageGraphTest.h"

#include <toucan/ImageGraph.h>
#include <toucan/Timeline.h>
#include <toucan/Util.h>

#include <sstream>

namespace toucan
{
    void imageGraphTest(
        const std::filesystem::path& path,
        const std::shared_ptr<ImageEffectHost>& host)
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
            std::shared_ptr<Timeline> timeline;
            try
            {
                timeline = std::make_shared<Timeline>(timelinePath);
            }
            catch (const std::exception& e)
            {
                std::cout << "ERROR: " << e.what() << std::endl;
                continue;
            }

            // Get time values.
            const OTIO_NS::TimeRange& timeRange = timeline->getTimeRange();
            const OTIO_NS::RationalTime timeInc(1.0, timeRange.duration().rate());

            // Render the timeline frames.
            const auto graph = std::make_shared<ImageGraph>(path, timeline);
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
                    const std::filesystem::path imagePath = getSequenceFrame(
                        std::filesystem::path(),
                        "imageGraphTest_" + otioFile + ".",
                        time.to_frames(),
                        6,
                        ".png");
                    buf.write(imagePath.string());
                }
            }
        }
    }
}
