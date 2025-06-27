// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/Action.h>
#include <feather-tk/ui/RowLayout.h>

namespace toucan
{
    class App;
    class MainWindow;

    //! Tool bar.
    class ToolBar : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<feather_tk::Action> >&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ToolBar();

        //! Create a new tool bar.
        static std::shared_ptr<ToolBar> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<feather_tk::Action> >&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::map<std::string, std::shared_ptr<feather_tk::IWidget> > _toolBars;
    };
}

