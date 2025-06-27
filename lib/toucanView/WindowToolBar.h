// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/WindowModel.h>

#include <feather-tk/ui/Action.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/ObservableList.h>

namespace toucan
{
    class App;
    class File;
    class MainWindow;

    //! Window tool bar.
    class WindowToolBar : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<feather_tk::Action> >&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~WindowToolBar();

        //! Create a new tool bar.
        static std::shared_ptr<WindowToolBar> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<feather_tk::Action> >&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::map<std::string, std::shared_ptr<feather_tk::ToolButton> > _buttons;

        std::shared_ptr<feather_tk::ValueObserver<bool> > _frameViewObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _fullScreenObserver;
        std::shared_ptr<feather_tk::MapObserver<WindowComponent, bool> > _componentObserver;
    };
}

