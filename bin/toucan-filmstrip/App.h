// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>
#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>

#include <feather-tk/core/IApp.h>

#include <OpenImageIO/imagebuf.h>

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
        struct Args
        {
            std::string input;
            std::string output;
        };
        Args _args;
        
        struct Options
        {
            bool verbose = false;
            bool help = false;
        };
        Options _options;

        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        std::shared_ptr<ImageGraph> _graph;
        std::shared_ptr<ImageEffectHost> _host;
    };
}
