// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/WindowModel.h>

#include <feather-tk/ui/Menu.h>
#include <feather-tk/core/ObservableList.h>
#include <feather-tk/core/ObservableMap.h>

namespace toucan
{
    class App;
    class MainWindow;

    //! Window menu.
    class WindowMenu : public feather_tk::Menu
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~WindowMenu();

        //! Create a new menu.
        static std::shared_ptr<WindowMenu> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the actions.
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& getActions() const;

    private:
        std::map<std::string, std::shared_ptr<feather_tk::Menu> > _menus;
        std::map<std::string, std::shared_ptr<feather_tk::Action> > _actions;

        std::shared_ptr<feather_tk::ValueObserver<bool> > _fullScreenObserver;
        std::shared_ptr<feather_tk::MapObserver<WindowComponent, bool> > _componentsObserver;
        std::shared_ptr<feather_tk::ValueObserver<float> > _displayScaleObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _thumbnailsObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _tooltipsObserver;
    };
}

