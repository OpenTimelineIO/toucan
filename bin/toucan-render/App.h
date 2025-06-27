// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>
#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>

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
                
        struct Args
        {
            std::string input;
            std::string output;
            bool outputRaw = false;
        };
        Args _args;
        
        struct Options
        {
            std::string videoCodec = "MJPEG";
            bool printStart = false;
            bool printDuration = false;
            bool printRate = false;
            bool printSize = false;
            std::string raw;
            std::string y4m;
            bool verbose = false;
        };
        Options _options;

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

