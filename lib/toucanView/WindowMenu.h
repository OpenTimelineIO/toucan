// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/WindowModel.h>

#include <ftk/Ui/Menu.h>
#include <ftk/Core/ObservableList.h>
#include <ftk/Core/ObservableMap.h>

namespace toucan
{
    class App;
    class MainWindow;

    //! Window menu.
    class WindowMenu : public ftk::Menu
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~WindowMenu();

        //! Create a new menu.
        static std::shared_ptr<WindowMenu> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the actions.
        const std::map<std::string, std::shared_ptr<ftk::Action> >& getActions() const;

    private:
        std::map<std::string, std::shared_ptr<ftk::Menu> > _menus;
        std::map<std::string, std::shared_ptr<ftk::Action> > _actions;

        std::shared_ptr<ftk::ValueObserver<bool> > _fullScreenObserver;
        std::shared_ptr<ftk::MapObserver<WindowComponent, bool> > _componentsObserver;
        std::shared_ptr<ftk::ValueObserver<float> > _displayScaleObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _thumbnailsObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _tooltipsObserver;
    };
}

