// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <dtk/ui/MenuBar.h>

namespace toucan
{
    class App;
    class MainWindow;

    //! Menu bar.
    class MenuBar : public dtk::MenuBar
    {
    protected:
        void _init(
            const std::shared_ptr<dtk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~MenuBar();

        //! Create a new menu bar.
        static std::shared_ptr<MenuBar> create(
            const std::shared_ptr<dtk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the actions.
        const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

    private:
        std::map<std::string, std::shared_ptr<dtk::Menu> > _menus;
        std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
    };
}

