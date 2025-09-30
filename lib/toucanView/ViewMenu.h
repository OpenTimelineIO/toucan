// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/ViewModel.h>

#include <ftk/Ui/Menu.h>
#include <ftk/Core/ObservableValue.h>

namespace toucan
{
    class App;
    class File;

    //! View menu.
    class ViewMenu : public ftk::Menu
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ViewMenu();

        //! Create a new menu.
        static std::shared_ptr<ViewMenu> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the actions.
        const std::map<std::string, std::shared_ptr<ftk::Action> >& getActions() const;

    private:
        void _menuUpdate();

        std::shared_ptr<File> _file;

        std::map<std::string, std::shared_ptr<ftk::Action> > _actions;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _frameViewObserver;
        std::shared_ptr<ftk::ValueObserver<ViewOptions> > _optionsObserver;
        std::shared_ptr<ftk::ValueObserver<GlobalViewOptions> > _globalOptionsObserver;
    };
}

