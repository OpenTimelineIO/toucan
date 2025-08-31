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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::IWidget::_init(context, "toucan::WindowToolBar", parent);

        _layout = ftk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        auto hLayout = ftk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);

        auto i = actions.find("Window/FullScreen");
        auto button = ftk::ToolButton::create(context, i->second, hLayout);
        _buttons["Window/FullScreen"] = button;

        i = actions.find("Window/Tools");
        button = ftk::ToolButton::create(context, i->second, hLayout);
        _buttons["Window/Tools"] = button;

        i = actions.find("Window/Playback");
        button = ftk::ToolButton::create(context, i->second, hLayout);
        _buttons["Window/Playback"] = button;

        _fullScreenObserver = ftk::ValueObserver<bool>::create(
            window->observeFullScreen(),
            [this](bool value)
            {
                _buttons["Window/FullScreen"]->setChecked(value);
            });

        _componentObserver = ftk::MapObserver<WindowComponent, bool>::create(
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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
        out->_init(context, app, window, actions, parent);
        return out;
    }

    void WindowToolBar::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void WindowToolBar::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
