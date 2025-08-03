// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>
#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>

#include <feather-tk/core/CmdLine.h>
#include <feather-tk/core/IApp.h>

#include <OpenImageIO/imagebuf.h>

extern "C"
{
#include <libswscale/swscale.h>

} // extern "C"

namespace toucan
{
    class App : public feather_tk::IApp
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            std::vector<std::string>&);

        App();

    public:
        ~App();
        
        static std::shared_ptr<App> create(
            const std::shared_ptr<feather_tk::Context>&,
            std::vector<std::string>&);

        void run() override;
    
    private:
        void _writeRawFrame(const OIIO::ImageBuf&);
        void _writeY4mHeader();
        void _writeY4mFrame(const OIIO::ImageBuf&);
        
        struct CmdLine
        {
            std::shared_ptr<feather_tk::CmdLineValueArg<std::string> > input;
            std::shared_ptr<feather_tk::CmdLineValueArg<std::string> > output;
            bool outputRaw = false;

            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > videoCodec;
            std::shared_ptr<feather_tk::CmdLineFlagOption> printStart;
            std::shared_ptr<feather_tk::CmdLineFlagOption> printDuration;
            std::shared_ptr<feather_tk::CmdLineFlagOption> printRate;
            std::shared_ptr<feather_tk::CmdLineFlagOption> printSize;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > raw;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > y4m;
            std::shared_ptr<feather_tk::CmdLineFlagOption> verbose;
        };
        CmdLine _cmdLine;

        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        std::shared_ptr<ImageGraph> _graph;
        std::shared_ptr<ImageEffectHost> _host;

        AVFrame* _avFrame = nullptr;
        AVFrame* _avFrame2 = nullptr;
        AVPixelFormat _avInputPixelFormat = AV_PIX_FMT_NONE;
        AVPixelFormat _avOutputPixelFormat = AV_PIX_FMT_NONE;
        SwsContext* _swsContext = nullptr;
    };
}

