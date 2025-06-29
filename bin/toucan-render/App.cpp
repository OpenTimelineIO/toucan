// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "App.h"

#include <toucanRender/FFmpegWrite.h>
#include <toucanRender/Read.h>
#include <toucanRender/Util.h>

#include <feather-tk/core/CmdLine.h>
#include <feather-tk/core/Time.h>

#include <OpenImageIO/imagebufalgo.h>

extern "C"
{
#include <libavutil/imgutils.h>

} // extern "C"

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

        const std::map<std::string, OIIO::ImageSpec> y4mSpecs =
        {
            { "422", OIIO::ImageSpec(0, 0, 3, OIIO::TypeDesc::BASETYPE::UINT8) },
            { "444", OIIO::ImageSpec(0, 0, 3, OIIO::TypeDesc::BASETYPE::UINT8) },
            { "444alpha", OIIO::ImageSpec(0, 0, 4, OIIO::TypeDesc::BASETYPE::UINT8) },
            { "444p16", OIIO::ImageSpec(0, 0, 3, OIIO::TypeDesc::BASETYPE::UINT16) }
        };
    }
    
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
            "Output image or movie file. Use a dash ('-') to write raw frames or y4m to stdout.");
        args.push_back(outArg);

        std::vector<std::shared_ptr<feather_tk::ICmdLineOption> > options;
        std::vector<std::string> rawList;
        for (const auto& spec : rawSpecs)
        {
            rawList.push_back(spec.first);
        }
        std::vector<std::string> y4mList;
        for (const auto& spec : y4mSpecs)
        {
            y4mList.push_back(spec.first);
        }
        options.push_back(feather_tk::CmdLineValueOption<std::string>::create(
            _options.videoCodec,
            std::vector<std::string>{ "-vcodec" },
            "Set the video codec.",
            _options.videoCodec,
            feather_tk::join(ffmpeg::getVideoCodecStrings(), ", ")));
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.printStart,
            std::vector<std::string>{ "-print_start" },
            "Print the timeline start time and exit."));
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.printDuration,
            std::vector<std::string>{ "-print_duration" },
            "Print the timeline duration and exit."));
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.printRate,
            std::vector<std::string>{ "-print_rate" },
            "Print the timeline frame rate and exit."));
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.printSize,
            std::vector<std::string>{ "-print_size" },
            "Print the timeline image size."));
        options.push_back(feather_tk::CmdLineValueOption<std::string>::create(
            _options.raw,
            std::vector<std::string>{ "-raw" },
            "Raw pixel format to send to stdout.",
            _options.raw,
            feather_tk::join(rawList, ", ")));
        options.push_back(feather_tk::CmdLineValueOption<std::string>::create(
            _options.y4m,
            std::vector<std::string>{ "-y4m" },
            "y4m format to send to stdout.",
            _options.y4m,
            feather_tk::join(y4mList, ", ")));
        options.push_back(feather_tk::CmdLineFlagOption::create(
            _options.verbose,
            std::vector<std::string>{ "-v" },
            "Print verbose output."));

        IApp::_init(
            context,
            argv,
            "toucan-render",
            "Render timeline files",
            args,
            options);

        _args.outputRaw = "-" == _args.output;
        if (_args.outputRaw)
        {
            _options.verbose = false;
        }
    }

    App::App()
    {}
        
    App::~App()
    {
        if (_swsContext)
        {
            sws_freeContext(_swsContext);
        }
        if (_avFrame2)
        {
            av_frame_free(&_avFrame2);
        }
        if (_avFrame)
        {
            av_frame_free(&_avFrame);
        }
    }

    std::shared_ptr<App> App::create(
        const std::shared_ptr<feather_tk::Context>& context,
        std::vector<std::string>& argv)
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

        // Print information.
        if (_options.printStart)
        {
            std::cout << timeRange.start_time().value() << std::endl;
            return;
        }
        else if (_options.printDuration)
        {
            std::cout << timeRange.duration().value() << std::endl;
            return;
        }
        else if (_options.printRate)
        {
            std::cout << timeRange.duration().rate() << std::endl;
            return;
        }
        else if (_options.printSize)
        {
            std::cout << imageSize.x << "x" << imageSize.y << std::endl;
            return;
        }

        // Create the image host.
        _host = std::make_shared<ImageEffectHost>(_context, getOpenFXPluginPaths(getExeName()));

        // Open the movie file.
        std::shared_ptr<ffmpeg::Write> ffWrite;
        if (MovieReadNode::hasExtension(outputPath.extension().string()))
        {
            ffmpeg::VideoCodec videoCodec = ffmpeg::VideoCodec::First;
            ffmpeg::fromString(_options.videoCodec, videoCodec);
            ffWrite = std::make_shared<ffmpeg::Write>(
                outputPath,
                OIIO::ImageSpec(imageSize.x, imageSize.y, 3),
                timeRange,
                videoCodec);
        }

        // Render the timeline frames.
        if (!_options.y4m.empty())
        {
            _writeY4mHeader();
        }
        for (OTIO_NS::RationalTime time = timeRange.start_time();
            time <= timeRange.end_time_inclusive();
            time += timeInc)
        {
            if (!_args.outputRaw)
            {
                std::cout << (time - timeRange.start_time()).value() << "/" <<
                    timeRange.duration().value() << std::endl;
            }

            if (auto node = _graph->exec(_host, time))
            {
                // Execute the graph.
                const auto buf = node->exec();

                // Save the image.
                if (!_args.outputRaw)
                {
                    if (ffWrite)
                    {
                        ffWrite->writeImage(buf, time);
                    }
                    else
                    {
                        const std::string fileName = getSequenceFrame(
                            outputPath.parent_path().string(),
                            outputSplit.first,
                            outputStartFrame + time.to_frames(),
                            outputNumberPadding,
                            outputPath.extension().string());
                        buf.write(fileName);
                    }
                }
                else if (!_options.raw.empty())
                {
                    _writeRawFrame(buf);
                }
                else if (!_options.y4m.empty())
                {
                    _writeY4mFrame(buf);
                }
            }
        }
    }

    void App::_writeRawFrame(const OIIO::ImageBuf& buf)
    {
        const OIIO::ImageBuf* p = &buf;
        auto spec = buf.spec();

        const auto i = rawSpecs.find(_options.raw);
        if (i == rawSpecs.end())
        {
            throw std::runtime_error("Cannot find the given raw format");
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

    void App::_writeY4mHeader()
    {
        std::string s = "YUV4MPEG2 ";
        fwrite(s.c_str(), s.size(), 1, stdout);

        {
            std::stringstream ss;
            ss << "W" << _graph->getImageSize().x;
            s = ss.str();
        }
        fwrite(s.c_str(), s.size(), 1, stdout);

        {
            std::stringstream ss;
            ss << " H" << _graph->getImageSize().y;
            s = ss.str();
        }
        fwrite(s.c_str(), s.size(), 1, stdout);

        {
            const OTIO_NS::TimeRange timeRange = _timelineWrapper->getTimeRange();
            const auto r = feather_tk::toRational(timeRange.duration().rate());
            std::stringstream ss;
            ss << " F" << r.first << ":" << r.second;
            s = ss.str();
        }
        fwrite(s.c_str(), s.size(), 1, stdout);

        {
            std::stringstream ss;
            ss << " C" << _options.y4m;
            s = ss.str();
        }
        fwrite(s.c_str(), s.size(), 1, stdout);

        fwrite("\n", 1, 1, stdout);
    }

    void App::_writeY4mFrame(const OIIO::ImageBuf& buf)
    {
        std::string s = "FRAME\n";
        fwrite(s.c_str(), s.size(), 1, stdout);

        const OIIO::ImageBuf* p = &buf;
        auto spec = buf.spec();

        const auto i = y4mSpecs.find(_options.y4m);
        if (i == y4mSpecs.end())
        {
            throw std::runtime_error("Cannot find the given y4m format");
        }
        auto y4mSpec = i->second;
        y4mSpec.width = spec.width;
        y4mSpec.height = spec.height;
        OIIO::ImageBuf tmp;
        if (spec.format != y4mSpec.format ||
            spec.nchannels != y4mSpec.nchannels)
        {
            spec = y4mSpec;
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

        if (!_swsContext)
        {
            if ("422" == _options.y4m)
            {
                _avInputPixelFormat = AV_PIX_FMT_RGB24;
                _avOutputPixelFormat = AV_PIX_FMT_YUV422P;
            }
            else if ("444" == _options.y4m)
            {
                _avInputPixelFormat = AV_PIX_FMT_RGB24;
                _avOutputPixelFormat = AV_PIX_FMT_YUV444P;
            }
            else if ("444alpha" == _options.y4m)
            {
                _avInputPixelFormat = AV_PIX_FMT_RGBA;
                _avOutputPixelFormat = AV_PIX_FMT_YUVA444P;
            }
            else if ("444p16" == _options.y4m)
            {
                _avInputPixelFormat = AV_PIX_FMT_RGB48;
                _avOutputPixelFormat = AV_PIX_FMT_YUV444P16;
            }

            _avFrame = av_frame_alloc();
            _avFrame->width = spec.width;
            _avFrame->height = spec.height;
            _avFrame->format = _avInputPixelFormat;
            av_frame_get_buffer(_avFrame, 1);

            _avFrame2 = av_frame_alloc();
            _avFrame2->width = y4mSpec.width;
            _avFrame2->height = y4mSpec.height;
            _avFrame2->format = _avOutputPixelFormat;
            av_frame_get_buffer(_avFrame2, 1);

            _swsContext = sws_getContext(
                spec.width,
                spec.height,
                _avInputPixelFormat,
                y4mSpec.width,
                y4mSpec.height,
                _avOutputPixelFormat,
                SWS_FAST_BILINEAR,
                nullptr,
                nullptr,
                nullptr);
        }

        memcpy(
            _avFrame->data[0],
            p->localpixels(),
            av_image_get_buffer_size(_avInputPixelFormat, spec.width, spec.height, 1));
        sws_scale_frame(_swsContext, _avFrame2, _avFrame);

        if ("422" == _options.y4m)
        {
            fwrite(
                _avFrame2->data[0],
                _avFrame2->linesize[0] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[1],
                _avFrame2->linesize[1] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[2],
                _avFrame2->linesize[2] * y4mSpec.height,
                1,
                stdout);
        }
        else if ("444" == _options.y4m || "444p16" == _options.y4m)
        {
            fwrite(
                _avFrame2->data[0],
                _avFrame2->linesize[0] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[1],
                _avFrame2->linesize[1] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[2],
                _avFrame2->linesize[2] * y4mSpec.height,
                1,
                stdout);
        }
        else if ("444alpha" == _options.y4m)
        {
            fwrite(
                _avFrame2->data[0],
                _avFrame2->linesize[0] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[1],
                _avFrame2->linesize[1] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[2],
                _avFrame2->linesize[2] * y4mSpec.height,
                1,
                stdout);
            fwrite(
                _avFrame2->data[3],
                _avFrame2->linesize[3] * y4mSpec.height,
                1,
                stdout);
        }
    }
}

