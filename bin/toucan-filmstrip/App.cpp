// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "App.h"

#include <toucanRender/Util.h>

#include <feather-tk/core/CmdLine.h>
#include <feather-tk/core/Time.h>

#include <OpenImageIO/imagebufalgo.h>

namespace toucan
{
    void App::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        std::vector<std::string>& argv)
    {
        std::vector<std::shared_ptr<feather_tk::ICmdLineArg> > args;
        args.push_back(feather_tk::CmdLineValueArg<std::string>::create(
            _args.input,
            "input",
            "Input .otio file."));
        auto outArg = feather_tk::CmdLineValueArg<std::string>::create(
            _args.output,
            "output",
            "Output image file.");
        args.push_back(outArg);

        std::vector<std::shared_ptr<feather_tk::ICmdLineOption> > options;
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.verbose,
            std::vector<std::string>{ "-v" },
            "Print verbose output."));
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.help,
            std::vector<std::string>{ "-h" },
            "Print help."));

        IApp::_init(
            context,
            argv,
            "toucan-filmstrip",
            "Render timeline files into filmstrips",
            args,
            options);
    }

    App::App()
    {}

    App::~App()
    {}

    std::shared_ptr<App> App::create(
        const std::shared_ptr<feather_tk::Context>&context,
        std::vector<std::string>&argv)
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(context, argv);
        return out;
    }
    
    void App::run()
    {
        const std::filesystem::path parentPath = std::filesystem::path(getExeName()).parent_path();
        const std::filesystem::path inputPath(_args.input);
        const std::filesystem::path outputPath(_args.output);
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

