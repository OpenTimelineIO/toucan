// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>
#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>

#include <feather-tk/core/IApp.h>
#include <feather-tk/core/CmdLine.h>

#include <OpenImageIO/imagebuf.h>

namespace toucan
{
    class App : public ftk::IApp
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            std::vector<std::string>&);

        App();

    public:
        ~App();

        static std::shared_ptr<App> create(
            const std::shared_ptr<ftk::Context>&,
            std::vector<std::string>&);
        
        void run() override;
    
    private:
        struct CmdLine
        {
            std::shared_ptr<ftk::CmdLineValueArg<std::string> > input;
            std::shared_ptr<ftk::CmdLineValueArg<std::string> > output;
        };
        CmdLine _cmdLine;

        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        std::shared_ptr<ImageGraph> _graph;
        std::shared_ptr<ImageEffectHost> _host;
    };
}
