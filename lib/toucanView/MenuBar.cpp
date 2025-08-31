// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MenuBar.h"

#include "CompareMenu.h"
#include "FileMenu.h"
#include "PlaybackMenu.h"
#include "SelectMenu.h"
#include "TimeMenu.h"
#include "ViewMenu.h"
#include "WindowMenu.h"

namespace toucan
{
    void MenuBar::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::MenuBar::_init(context, parent);

        auto fileMenu = FileMenu::create(context, app, window);
        addMenu("File", fileMenu);
        _menus["File"] = fileMenu;
        _actions.insert(fileMenu->getActions().begin(), fileMenu->getActions().end());

        auto compareMenu = CompareMenu::create(context, app);
        addMenu("Compare", compareMenu);
        _menus["Compare"] = compareMenu;
        _actions.insert(compareMenu->getActions().begin(), compareMenu->getActions().end());

        auto selectMenu = SelectMenu::create(context, app);
        addMenu("Select", selectMenu);
        _menus["Select"] = selectMenu;
        _actions.insert(selectMenu->getActions().begin(), selectMenu->getActions().end());

        auto timeMenu = TimeMenu::create(context, app);
        addMenu("Time", timeMenu);
        _menus["Time"] = timeMenu;
        _actions.insert(timeMenu->getActions().begin(), timeMenu->getActions().end());

        auto playbackMenu = PlaybackMenu::create(context, app);
        addMenu("Playback", playbackMenu);
        _menus["Playback"] = playbackMenu;
        _actions.insert(playbackMenu->getActions().begin(), playbackMenu->getActions().end());

        auto viewMenu = ViewMenu::create(context, app);
        addMenu("View", viewMenu);
        _menus["View"] = viewMenu;
        _actions.insert(viewMenu->getActions().begin(), viewMenu->getActions().end());

        auto windowMenu = WindowMenu::create(context, app, window);
        addMenu("Window", windowMenu);
        _menus["Window"] = windowMenu;
        _actions.insert(windowMenu->getActions().begin(), windowMenu->getActions().end());
    }

    MenuBar::~MenuBar()
    {}

    std::shared_ptr<MenuBar> MenuBar::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<MenuBar>(new MenuBar);
        out->_init(context, app, window, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& MenuBar::getActions() const
    {
        return _actions;
    }
}
