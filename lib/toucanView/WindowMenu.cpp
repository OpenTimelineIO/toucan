// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "WindowMenu.h"

#include "App.h"
#include "FilesModel.h"
#include "MainWindow.h"
#include "SelectionModel.h"
#include "ViewModel.h"

#include <toucanRender/TimelineAlgo.h>

#include <dtk/ui/Action.h>
#include <dtk/ui/FileBrowser.h>
#include <dtk/ui/RecentFilesModel.h>
#include <dtk/core/Format.h>

namespace toucan
{
    void WindowMenu::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        dtk::Menu::_init(context, parent);

        std::weak_ptr<MainWindow> windowWeak(window);
        _actions["Window/FullScreen"] = std::make_shared<dtk::Action>(
            "Full Screen",
            "WindowFullScreen",
            dtk::Key::U,
            static_cast<int>(dtk::commandKeyModifier),
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setFullScreen(value);
                }
            });
        _actions["Window/FullScreen"]->toolTip = "Toggle full screen mode";
        addItem(_actions["Window/FullScreen"]);

        addDivider();

        struct Component
        {
            WindowComponent component;
            std::string action;
            std::string text;
            std::string icon;
            std::string tooltip;
        };
        const std::vector<Component> components =
        {
            { WindowComponent::ToolBar, "ToolBar", "Tool Bar", "", "" },
            { WindowComponent::Tools, "Tools", "Tools", "PanelRight", "Toggle the tools" },
            { WindowComponent::Playback, "Playback", "Playback", "PanelBottom", "Toggle the playback controls" },
            { WindowComponent::InfoBar, "InfoBar", "InfoBar", "", "Toggle the information bar" }
        };
        std::weak_ptr<App> appWeak(app);
        for (const auto& component : components)
        {
            const std::string actionName = dtk::Format("Window/{0}").arg(component.action);
            _actions[actionName] = std::make_shared<dtk::Action>(
                component.text,
                component.icon,
                [appWeak, component](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getWindowModel()->setComponent(component.component, value);
                    }
                });
            _actions[actionName]->toolTip = component.tooltip;
            addItem(_actions[actionName]);
        }

        addDivider();

        _menus["Window/Resize"] = addSubMenu("Resize");

        _actions["Window/Resize/1280x720"] = std::make_shared<dtk::Action>(
            "1280x720",
            [windowWeak]
            {
                if (auto window = windowWeak.lock())
                {
                    window->setSize(dtk::Size2I(1280, 720));
                }
            });
        _menus["Window/Resize"]->addItem(_actions["Window/Resize/1280x720"]);

        _actions["Window/Resize/1920x1080"] = std::make_shared<dtk::Action>(
            "1920x1080",
            [windowWeak]
            {
                if (auto window = windowWeak.lock())
                {
                    window->setSize(dtk::Size2I(1920, 1080));
                }
            });
        _menus["Window/Resize"]->addItem(_actions["Window/Resize/1920x1080"]);

        _actions["Window/Resize/3840x2160"] = std::make_shared<dtk::Action>(
            "3840x2160",
            [windowWeak]
            {
                if (auto window = windowWeak.lock())
                {
                    window->setSize(dtk::Size2I(3840, 2160));
                }
            });
        _menus["Window/Resize"]->addItem(_actions["Window/Resize/3840x2160"]);

        _menus["Window/DisplayScale"] = addSubMenu("Display Scale");

        _actions["Window/DisplayScale/Auto"] = std::make_shared<dtk::Action>(
            "Automatic",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(0.F);
                }
            });
        _menus["Window/DisplayScale"]->addItem(_actions["Window/DisplayScale/Auto"]);

        _actions["Window/DisplayScale/1.0"] = std::make_shared<dtk::Action>(
            "1.0",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(1.F);
                }
            });
        _menus["Window/DisplayScale"]->addItem(_actions["Window/DisplayScale/1.0"]);

        _actions["Window/DisplayScale/1.5"] = std::make_shared<dtk::Action>(
            "1.5",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(1.5F);
                }
            });
        _menus["Window/DisplayScale"]->addItem(_actions["Window/DisplayScale/1.5"]);

        _actions["Window/DisplayScale/2.0"] = std::make_shared<dtk::Action>(
            "2.0",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(2.F);
                }
            });
        _menus["Window/DisplayScale"]->addItem(_actions["Window/DisplayScale/2.0"]);

        _actions["Window/DisplayScale/2.5"] = std::make_shared<dtk::Action>(
            "2.5",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(2.5F);
                }
            });
        _menus["Window/DisplayScale"]->addItem(_actions["Window/DisplayScale/2.5"]);

        _actions["Window/DisplayScale/3.0"] = std::make_shared<dtk::Action>(
            "3.0",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(3.F);
                }
            });
        _menus["Window/DisplayScale"]->addItem(_actions["Window/DisplayScale/3.0"]);

        _actions["Window/Thumbnails"] = std::make_shared<dtk::Action>(
            "Thumbnails",
            [appWeak](bool value)
            {
                if (auto app = appWeak.lock())
                {
                    app->getWindowModel()->setThumbnails(value);
                }
            });
        addItem(_actions["Window/Thumbnails"]);

        _actions["Window/Tooltips"] = std::make_shared<dtk::Action>(
            "Tooltips",
            [appWeak](bool value)
            {
                if (auto app = appWeak.lock())
                {
                    app->getWindowModel()->setTooltips(value);
                }
            });
        addItem(_actions["Window/Tooltips"]);

        _fullScreenObserver = dtk::ValueObserver<bool>::create(
            window->observeFullScreen(),
            [this](bool value)
            {
                setItemChecked(_actions["Window/FullScreen"], value);
            });

        _componentsObserver = dtk::MapObserver<WindowComponent, bool>::create(
            app->getWindowModel()->observeComponents(),
            [this](const std::map<WindowComponent, bool>& value)
            {
                auto i = value.find(WindowComponent::ToolBar);
                setItemChecked(_actions["Window/ToolBar"], i->second);
                i = value.find(WindowComponent::Tools);
                setItemChecked(_actions["Window/Tools"], i->second);
                i = value.find(WindowComponent::Playback);
                setItemChecked(_actions["Window/Playback"], i->second);
                i = value.find(WindowComponent::InfoBar);
                setItemChecked(_actions["Window/InfoBar"], i->second);
            });

        _displayScaleObserver = dtk::ValueObserver<float>::create(
            window->observeDisplayScale(),
            [this](float value)
            {
                _menus["Window/DisplayScale"]->setItemChecked(_actions["Window/DisplayScale/Auto"], 0.F == value);
                _menus["Window/DisplayScale"]->setItemChecked(_actions["Window/DisplayScale/1.0"], 1.F == value);
                _menus["Window/DisplayScale"]->setItemChecked(_actions["Window/DisplayScale/1.5"], 1.5F == value);
                _menus["Window/DisplayScale"]->setItemChecked(_actions["Window/DisplayScale/2.0"], 2.F == value);
                _menus["Window/DisplayScale"]->setItemChecked(_actions["Window/DisplayScale/2.5"], 2.5F == value);
                _menus["Window/DisplayScale"]->setItemChecked(_actions["Window/DisplayScale/3.0"], 3.F == value);
            });

        _thumbnailsObserver = dtk::ValueObserver<bool>::create(
            app->getWindowModel()->observeThumbnails(),
            [this](bool value)
            {
                setItemChecked(_actions["Window/Thumbnails"], value);
            });

        _tooltipsObserver = dtk::ValueObserver<bool>::create(
            app->getWindowModel()->observeTooltips(),
            [this](bool value)
            {
                setItemChecked(_actions["Window/Tooltips"], value);
            });
    }

    WindowMenu::~WindowMenu()
    {}

    std::shared_ptr<WindowMenu> WindowMenu::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
        out->_init(context, app, window, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<dtk::Action> >& WindowMenu::getActions() const
    {
        return _actions;
    }
}
