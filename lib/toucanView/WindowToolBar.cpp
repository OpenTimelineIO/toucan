// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "WindowToolBar.h"

#include "App.h"
#include "FilesModel.h"
#include "MainWindow.h"
#include "ViewModel.h"

namespace toucan
{
    void WindowToolBar::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::WindowToolBar", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        auto hLayout = feather_tk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        auto i = actions.find("Window/FullScreen");
        auto button = feather_tk::ToolButton::create(context, i->second, hLayout);
        _buttons["Window/FullScreen"] = button;

        i = actions.find("Window/Tools");
        button = feather_tk::ToolButton::create(context, i->second, hLayout);
        _buttons["Window/Tools"] = button;

        i = actions.find("Window/Playback");
        button = feather_tk::ToolButton::create(context, i->second, hLayout);
        _buttons["Window/Playback"] = button;

        _fullScreenObserver = feather_tk::ValueObserver<bool>::create(
            window->observeFullScreen(),
            [this](bool value)
            {
                _buttons["Window/FullScreen"]->setChecked(value);
            });

        _componentObserver = feather_tk::MapObserver<WindowComponent, bool>::create(
            app->getWindowModel()->observeComponents(),
            [this](const std::map<WindowComponent, bool> value)
            {
                auto i = value.find(WindowComponent::Tools);
                _buttons["Window/Tools"]->setChecked(i != value.end() ? i->second : false);
                i = value.find(WindowComponent::Playback);
                _buttons["Window/Playback"]->setChecked(i != value.end() ? i->second : false);
            });
    }

    WindowToolBar::~WindowToolBar()
    {}

    std::shared_ptr<WindowToolBar> WindowToolBar::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
        out->_init(context, app, window, actions, parent);
        return out;
    }

    void WindowToolBar::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void WindowToolBar::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
