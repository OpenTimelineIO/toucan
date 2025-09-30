// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "WindowMenu.h"

#include "App.h"
#include "FilesModel.h"
#include "MainWindow.h"
#include "SelectionModel.h"
#include "ViewModel.h"

#include <toucanRender/TimelineAlgo.h>

#include <ftk/Ui/Action.h>
#include <ftk/Ui/FileBrowser.h>
#include <ftk/Ui/RecentFilesModel.h>
#include <ftk/Core/Format.h>

namespace toucan
{
    void WindowMenu::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::Menu::_init(context, parent);

        std::weak_ptr<MainWindow> windowWeak(window);
        _actions["Window/FullScreen"] = ftk::Action::create(
            "Full Screen",
            "WindowFullScreen",
            ftk::Key::U,
            static_cast<int>(ftk::commandKeyModifier),
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setFullScreen(value);
                }
            });
        _actions["Window/FullScreen"]->setTooltip("Toggle full screen mode");
        addAction(_actions["Window/FullScreen"]);

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
            const std::string actionName = ftk::Format("Window/{0}").arg(component.action);
            _actions[actionName] = ftk::Action::create(
                component.text,
                component.icon,
                [appWeak, component](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getWindowModel()->setComponent(component.component, value);
                    }
                });
            _actions[actionName]->setTooltip(component.tooltip);
            addAction(_actions[actionName]);
        }

        addDivider();

        _menus["Window/Resize"] = addSubMenu("Resize");

        _actions["Window/Resize/1280x720"] = ftk::Action::create(
            "1280x720",
            [windowWeak]
            {
                if (auto window = windowWeak.lock())
                {
                    window->setSize(ftk::Size2I(1280, 720));
                }
            });
        _menus["Window/Resize"]->addAction(_actions["Window/Resize/1280x720"]);

        _actions["Window/Resize/1920x1080"] = ftk::Action::create(
            "1920x1080",
            [windowWeak]
            {
                if (auto window = windowWeak.lock())
                {
                    window->setSize(ftk::Size2I(1920, 1080));
                }
            });
        _menus["Window/Resize"]->addAction(_actions["Window/Resize/1920x1080"]);

        _actions["Window/Resize/3840x2160"] = ftk::Action::create(
            "3840x2160",
            [windowWeak]
            {
                if (auto window = windowWeak.lock())
                {
                    window->setSize(ftk::Size2I(3840, 2160));
                }
            });
        _menus["Window/Resize"]->addAction(_actions["Window/Resize/3840x2160"]);

        _menus["Window/DisplayScale"] = addSubMenu("Display Scale");

        _actions["Window/DisplayScale/1.0"] = ftk::Action::create(
            "1.0",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(1.F);
                }
            });
        _menus["Window/DisplayScale"]->addAction(_actions["Window/DisplayScale/1.0"]);

        _actions["Window/DisplayScale/1.5"] = ftk::Action::create(
            "1.5",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(1.5F);
                }
            });
        _menus["Window/DisplayScale"]->addAction(_actions["Window/DisplayScale/1.5"]);

        _actions["Window/DisplayScale/2.0"] = ftk::Action::create(
            "2.0",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(2.F);
                }
            });
        _menus["Window/DisplayScale"]->addAction(_actions["Window/DisplayScale/2.0"]);

        _actions["Window/DisplayScale/2.5"] = ftk::Action::create(
            "2.5",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(2.5F);
                }
            });
        _menus["Window/DisplayScale"]->addAction(_actions["Window/DisplayScale/2.5"]);

        _actions["Window/DisplayScale/3.0"] = ftk::Action::create(
            "3.0",
            [windowWeak](bool value)
            {
                if (auto window = windowWeak.lock())
                {
                    window->setDisplayScale(3.F);
                }
            });
        _menus["Window/DisplayScale"]->addAction(_actions["Window/DisplayScale/3.0"]);

        _actions["Window/Thumbnails"] = ftk::Action::create(
            "Thumbnails",
            [appWeak](bool value)
            {
                if (auto app = appWeak.lock())
                {
                    app->getWindowModel()->setThumbnails(value);
                }
            });
        addAction(_actions["Window/Thumbnails"]);

        _actions["Window/Tooltips"] = ftk::Action::create(
            "Tooltips",
            [appWeak](bool value)
            {
                if (auto app = appWeak.lock())
                {
                    app->getWindowModel()->setTooltips(value);
                }
            });
        addAction(_actions["Window/Tooltips"]);

        _fullScreenObserver = ftk::ValueObserver<bool>::create(
            window->observeFullScreen(),
            [this](bool value)
            {
                setChecked(_actions["Window/FullScreen"], value);
            });

        _componentsObserver = ftk::MapObserver<WindowComponent, bool>::create(
            app->getWindowModel()->observeComponents(),
            [this](const std::map<WindowComponent, bool>& value)
            {
                auto i = value.find(WindowComponent::ToolBar);
                setChecked(_actions["Window/ToolBar"], i->second);
                i = value.find(WindowComponent::Tools);
                setChecked(_actions["Window/Tools"], i->second);
                i = value.find(WindowComponent::Playback);
                setChecked(_actions["Window/Playback"], i->second);
                i = value.find(WindowComponent::InfoBar);
                setChecked(_actions["Window/InfoBar"], i->second);
            });

        _displayScaleObserver = ftk::ValueObserver<float>::create(
            window->observeDisplayScale(),
            [this](float value)
            {
                _menus["Window/DisplayScale"]->setChecked(_actions["Window/DisplayScale/1.0"], 1.F == value);
                _menus["Window/DisplayScale"]->setChecked(_actions["Window/DisplayScale/1.5"], 1.5F == value);
                _menus["Window/DisplayScale"]->setChecked(_actions["Window/DisplayScale/2.0"], 2.F == value);
                _menus["Window/DisplayScale"]->setChecked(_actions["Window/DisplayScale/2.5"], 2.5F == value);
                _menus["Window/DisplayScale"]->setChecked(_actions["Window/DisplayScale/3.0"], 3.F == value);
            });

        _thumbnailsObserver = ftk::ValueObserver<bool>::create(
            app->getWindowModel()->observeThumbnails(),
            [this](bool value)
            {
                setChecked(_actions["Window/Thumbnails"], value);
            });

        _tooltipsObserver = ftk::ValueObserver<bool>::create(
            app->getWindowModel()->observeTooltips(),
            [this](bool value)
            {
                setChecked(_actions["Window/Tooltips"], value);
            });
    }

    WindowMenu::~WindowMenu()
    {}

    std::shared_ptr<WindowMenu> WindowMenu::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
        out->_init(context, app, window, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& WindowMenu::getActions() const
    {
        return _actions;
    }
}
