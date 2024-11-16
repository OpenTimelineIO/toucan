// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "App.h"

#include "Util.h"

#include <toucan/ImageEffectHost.h>
#include <toucan/ImageGraph.h>
#include <toucan/TimelineWrapper.h>
#include <toucan/Util.h>

#include <OpenImageIO/imagebufalgo.h>

#include <stdio.h>

namespace toucan
{
    namespace
    {
        const std::map<std::string, OIIO::ImageSpec> rawSpecs =
        {
            { "rgb24", OIIO::ImageSpec(0, 0, 3, OIIO::TypeDesc::BASETYPE::UINT8) },
            { "rgba", OIIO::ImageSpec(0, 0, 4, OIIO::TypeDesc::BASETYPE::UINT8) },
            { "rgb48", OIIO::ImageSpec(0, 0, 3, OIIO::TypeDesc::BASETYPE::UINT16) },
            { "rgba64", OIIO::ImageSpec(0, 0, 4, OIIO::TypeDesc::BASETYPE::UINT16) },
            { "rgbaf16", OIIO::ImageSpec(0, 0, 4, OIIO::TypeDesc::BASETYPE::HALF) },
            { "rgbf32", OIIO::ImageSpec(0, 0, 3, OIIO::TypeDesc::BASETYPE::FLOAT) },
            { "rgbaf32", OIIO::ImageSpec(0, 0, 4, OIIO::TypeDesc::BASETYPE::FLOAT) }
        };
    }
    
    App::App(std::vector<std::string>& argv)
    {
        _exe = argv.front();
        argv.erase(argv.begin());

        _args.list.push_back(std::make_shared<CmdLineValueArg<std::string> >(
            _args.input,
            "input",
            "Input .otio file."));
        auto outArg = std::make_shared<CmdLineValueArg<std::string> >(
            _args.output,
            "output",
            "Output image file. Use a dash ('-') to write raw frames to stdout.");
        _args.list.push_back(outArg);

        std::vector<std::string> rawList;
        for (const auto& spec : rawSpecs)
        {
            rawList.push_back(spec.first);
        }
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.printStart,
            std::vector<std::string>{ "-print_start" },
            "Print the timeline start time and exit."));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.printDuration,
            std::vector<std::string>{ "-print_duration" },
            "Print the timeline duration and exit."));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.printRate,
            std::vector<std::string>{ "-print_rate" },
            "Print the timeline frame rate and exit."));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.printSize,
            std::vector<std::string>{ "-print_size" },
            "Print the timeline image size."));
        _options.list.push_back(std::make_shared<CmdLineValueOption<std::string> >(
            _options.raw,
            std::vector<std::string>{ "-raw" },
            "Raw pixel format to output.",
            _options.raw,
            join(rawList, ", ")));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.filmstrip,
            std::vector<std::string>{ "-filmstrip" },
            "Render the frames to a single output image as thumbnails in a row."));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.graph,
            std::vector<std::string>{ "-graph" },
            "Write a Graphviz graph for each frame."));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.verbose,
            std::vector<std::string>{ "-v" },
            "Print verbose output."));
        _options.list.push_back(std::make_shared<CmdLineFlagOption>(
            _options.help,
            std::vector<std::string>{ "-h" },
            "Print help."));

        if (!argv.empty())
        {
            for (const auto& option : _options.list)
            {
                option->parse(argv);
            }
            if (_options.printStart ||
                _options.printDuration ||
                _options.printRate ||
                _options.printSize)
            {
                auto i = std::find(_args.list.begin(), _args.list.end(), outArg);
                if (i != _args.list.end())
                {
                    _args.list.erase(i);
                }
            }
            if (!_options.help)
            {
                for (const auto& arg : _args.list)
                {
                    arg->parse(argv);
                }
                _args.outputRaw = "-" == _args.output;
                if (_args.outputRaw)
                {
                    _options.verbose = false;
                }
                if (argv.size())
                {
                    _options.help = true;
                }
            }
        }
        else
        {
            _options.help = true;
        }
    }
        
    App::~App()
    {}
    
    int App::run()
    {
        if (_options.help)
        {
            _printHelp();
            return 1;
        }
        
        const std::filesystem::path parentPath = std::filesystem::path(_exe).parent_path();
        const std::filesystem::path inputPath(_args.input);
        const std::filesystem::path outputPath(_args.output);
        const auto outputSplit = splitFileNameNumber(outputPath.stem().string());
        const int outputStartFrame = atoi(outputSplit.second.c_str());
        const size_t outputNumberPadding = getNumberPadding(outputSplit.second);

        // Open the timeline.
        auto timelineWrapper = std::make_shared<TimelineWrapper>(inputPath);

        // Get time values.
        const OTIO_NS::TimeRange& timeRange = timelineWrapper->getTimeRange();
        const OTIO_NS::RationalTime timeInc(1.0, timeRange.duration().rate());
        const int frames = timeRange.duration().value();
        
        // Create the image graph.
        std::shared_ptr<MessageLog> log;
        if (_options.verbose)
        {
            log = std::make_shared<MessageLog>();
        }
        ImageGraphOptions imageGraphOptions;
        imageGraphOptions.log = log;
        const auto graph = std::make_shared<ImageGraph>(
            inputPath.parent_path(),
            timelineWrapper,
            imageGraphOptions);
        const IMATH_NAMESPACE::V2d imageSize = graph->getImageSize();

        // Print information.
        if (_options.printStart)
        {
            std::cout << timeRange.start_time().value() << std::endl;
            return 0;
        }
        else if (_options.printDuration)
        {
            std::cout << timeRange.duration().value() << std::endl;
            return 0;
        }
        else if (_options.printRate)
        {
            std::cout << timeRange.duration().rate() << std::endl;
            return 0;
        }
        else if (_options.printSize)
        {
            std::cout << imageSize.x << "x" << imageSize.y << std::endl;
            return 0;
        }

        // Create the image host.
        std::vector<std::filesystem::path> searchPath;
        searchPath.push_back(parentPath);
#if defined(_WINDOWS)
        searchPath.push_back(parentPath / ".." / ".." / "..");
#else // _WINDOWS
        searchPath.push_back(parentPath / ".." / "..");
#endif // _WINDOWS
        ImageEffectHostOptions imageHostOptions;
        imageHostOptions.log = log;
        auto host = std::make_shared<ImageEffectHost>(
            searchPath,
            imageHostOptions);

        // Initialize the filmstrip.
        OIIO::ImageBuf filmstripBuf;
        const int thumbnailWidth = 360;
        const int thumbnailSpacing = 0;
        IMATH_NAMESPACE::V2d thumbnailSize;
        if (_options.filmstrip && imageSize.x > 0 && imageSize.y > 0)
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
            if (!_args.outputRaw)
            {
                std::cout << (time - timeRange.start_time()).value() << "/" <<
                    timeRange.duration().value() << std::endl;
            }

            if (auto node = graph->exec(host, time))
            {
                // Execute the graph.
                const auto buf = node->exec();

                // Save the image.
                if (!_options.filmstrip)
                {
                    if (!_args.outputRaw)
                    {
                        const std::string fileName = getSequenceFrame(
                            outputPath.parent_path().string(),
                            outputSplit.first,
                            outputStartFrame + time.to_frames(),
                            outputNumberPadding,
                            outputPath.extension().string());
                        buf.write(fileName);
                    }
                    else
                    {
                        _writeRaw(buf);
                    }
                }
                else
                {
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

                // Write the graph.
                if (_options.graph)
                {
                    const std::string fileName = getSequenceFrame(
                        outputPath.parent_path().string(),
                        outputSplit.first,
                        outputStartFrame + time.to_frames(),
                        outputNumberPadding,
                        ".dot");
                    const std::vector<std::string> lines = node->graph(inputPath.stem().string());
                    if (FILE* f = fopen(fileName.c_str(), "w"))
                    {
                        for (const auto& line : lines)
                        {
                            fprintf(f, "%s\n", line.c_str());
                        }
                        fclose(f);
                    }
                }
            }
        }
        if (_options.filmstrip)
        {
            if (!_args.outputRaw)
            {
                filmstripBuf.write(outputPath.string());
            }
            else
            {
                _writeRaw(filmstripBuf);
            }
        }
    
        return 0;
    }

    void App::_writeRaw(const OIIO::ImageBuf& buf)
    {
        const OIIO::ImageBuf* p = &buf;
        auto spec = buf.spec();

        const auto i = rawSpecs.find(_options.raw);
        if (i == rawSpecs.end())
        {
            throw std::runtime_error("Cannot find given raw pixel format");
        }
        auto rawSpec = i->second;
        rawSpec.width = spec.width;
        rawSpec.height = spec.height;
        OIIO::ImageBuf tmp;
        if (spec.format != rawSpec.format)
        {
            spec = rawSpec;
            tmp = OIIO::ImageBuf(spec);
            OIIO::ImageBufAlgo::paste(
                tmp,
                0,
                0,
                0,
                0,
                buf);
            p = &tmp;
        }

        fwrite(
            p->localpixels(),
            spec.image_bytes(),
            1,
            stdout);
    }
    
    void App::_printHelp()
    {
        std::cout << "Usage:" << std::endl;
        std::cout << "    toucan-render (input) (output) [options...]" << std::endl;
        std::cout << "    toucan-render (input) (-print_start|-print_duration|-print_rate|-print_size)" << std::endl;
        std::cout << std::endl;
        std::cout << "Arguments:" << std::endl;
        for (const auto& arg : _args.list)
        {
            std::cout << "    " << arg->getName() << std::endl;
            std::cout << "    " << arg->getHelp() << std::endl;
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        for (const auto& option : _options.list)
        {
            for (const auto& line : option->getHelp())
            {
                std::cout << "    " << line << std::endl;
            }
            std::cout << std::endl;
        }        
    }
}

