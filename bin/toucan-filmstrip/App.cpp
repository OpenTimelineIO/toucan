// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "App.h"

#include <toucanRender/Util.h>

#include <ftk/Core/Time.h>

#include <OpenImageIO/imagebufalgo.h>

namespace toucan
{
    void App::_init(
        const std::shared_ptr<ftk::Context>& context,
        std::vector<std::string>& argv)
    {
        _cmdLine.input = ftk::CmdLineValueArg<std::string>::create(
            "input",
            "Input .otio file.");
        _cmdLine.output = ftk::CmdLineValueArg<std::string>::create(
            "output",
            "Output image file.");

        IApp::_init(
            context,
            argv,
            "toucan-filmstrip",
            "Render timeline files into filmstrips",
            { _cmdLine.input, _cmdLine.output });
    }

    App::App()
    {}

    App::~App()
    {}

    std::shared_ptr<App> App::create(
        const std::shared_ptr<ftk::Context>&context,
        std::vector<std::string>&argv)
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(context, argv);
        return out;
    }
    
    void App::run()
    {
        const std::filesystem::path parentPath = std::filesystem::path(getExeName()).parent_path();
        const std::filesystem::path inputPath(_cmdLine.input->getValue());
        const std::filesystem::path outputPath(_cmdLine.output->getValue());
        const auto outputSplit = splitFileNameNumber(outputPath.stem().string());
        const int outputStartFrame = atoi(outputSplit.second.c_str());
        const size_t outputNumberPadding = getNumberPadding(outputSplit.second);

        // Open the timeline.
        _timelineWrapper = std::make_shared<TimelineWrapper>(inputPath);

        // Get time values.
        const OTIO_NS::TimeRange& timeRange = _timelineWrapper->getTimeRange();
        const OTIO_NS::RationalTime timeInc(1.0, timeRange.duration().rate());
        const int frames = timeRange.duration().value();
        
        // Create the image graph.
        _graph = std::make_shared<ImageGraph>(
            _context,
            inputPath.parent_path(),
            _timelineWrapper);
        const IMATH_NAMESPACE::V2d imageSize = _graph->getImageSize();

        // Create the image host.
        _host = std::make_shared<ImageEffectHost>(_context, getOpenFXPluginPaths(getExeName()));

        // Initialize the filmstrip.
        OIIO::ImageBuf filmstripBuf;
        const int thumbnailWidth = 360;
        const int thumbnailSpacing = 0;
        IMATH_NAMESPACE::V2d thumbnailSize;
        if (imageSize.x > 0 && imageSize.y > 0)
        {
            thumbnailSize = IMATH_NAMESPACE::V2d(
                thumbnailWidth,
                thumbnailWidth / static_cast<float>(imageSize.x / static_cast<float>(imageSize.y)));
            const IMATH_NAMESPACE::V2d filmstripSize(
                thumbnailSize.x * frames + thumbnailSpacing * (frames - 1),
                thumbnailSize.y);
            filmstripBuf = OIIO::ImageBufAlgo::fill(
                { 0.F, 0.F, 0.F, 0.F },
                OIIO::ROI(0, filmstripSize.x, 0, filmstripSize.y, 0, 1, 0, 4));
        }

        // Render the timeline frames.
        int filmstripX = 0;
        for (OTIO_NS::RationalTime time = timeRange.start_time();
            time <= timeRange.end_time_inclusive();
            time += timeInc)
        {
            std::cout << (time - timeRange.start_time()).value() << "/" <<
                timeRange.duration().value() << std::endl;

            if (auto node = _graph->exec(_host, time))
            {
                // Execute the graph.
                const auto buf = node->exec();

                // Append the image.
                const auto thumbnailBuf = OIIO::ImageBufAlgo::resize(
                    buf,
                    "",
                    0.0,
                    OIIO::ROI(0, thumbnailSize.x, 0, thumbnailSize.y, 0, 1, 0, 4));
                OIIO::ImageBufAlgo::paste(
                    filmstripBuf,
                    filmstripX,
                    0,
                    0,
                    0,
                    thumbnailBuf);
                filmstripX += thumbnailSize.x + thumbnailSpacing;
            }
        }

        // Write the image.
        filmstripBuf.write(outputPath.string());
    }
}

