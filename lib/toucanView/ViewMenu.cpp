// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ViewMenu.h"

#include "App.h"
#include "FilesModel.h"

namespace toucan
{
    void ViewMenu::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::Menu::_init(context, parent);

        _actions["View/ZoomIn"] = ftk::Action::create(
            "Zoom In",
            "ViewZoomIn",
            ftk::Key::Equals,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getViewModel()->zoomIn();
                }
            });
        _actions["View/ZoomIn"]->setTooltip("View zoom in");
        addAction(_actions["View/ZoomIn"]);

        _actions["View/ZoomOut"] = ftk::Action::create(
            "Zoom Out",
            "ViewZoomOut",
            ftk::Key::Minus,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getViewModel()->zoomOut();
                }
            });
        _actions["View/ZoomOut"]->setTooltip("View zoom out");
        addAction(_actions["View/ZoomOut"]);

        _actions["View/ZoomReset"] = ftk::Action::create(
            "Zoom Reset",
            "ViewZoomReset",
            ftk::Key::_0,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getViewModel()->zoomReset();
                }
            });
        _actions["View/ZoomReset"]->setTooltip("Reset the view zoom");
        addAction(_actions["View/ZoomReset"]);

        addDivider();

        _actions["View/Frame"] = ftk::Action::create(
            "Frame View",
            "ViewFrame",
            ftk::Key::Backspace,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    _file->getViewModel()->setFrameView(value);
                }
            });
        _actions["View/Frame"]->setTooltip("Frame the view");
        addAction(_actions["View/Frame"]);

        addDivider();

        _actions["View/Flip"] = ftk::Action::create(
            "Flip Vertical",
            ftk::Key::V,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.flip = value;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Flip"]);

        _actions["View/Flop"] = ftk::Action::create(
            "Flop Horizontal",
            ftk::Key::H,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.flop = value;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Flop"]);

        addDivider();

        _actions["View/Red"] = ftk::Action::create(
            "Red",
            ftk::Key::R,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? ftk::ChannelDisplay::Red : ftk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Red"]);

        _actions["View/Green"] = ftk::Action::create(
            "Green",
            ftk::Key::G,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? ftk::ChannelDisplay::Green : ftk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Green"]);

        _actions["View/Blue"] = ftk::Action::create(
            "Blue",
            ftk::Key::B,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? ftk::ChannelDisplay::Blue : ftk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Blue"]);

        _actions["View/Alpha"] = ftk::Action::create(
            "Alpha",
            ftk::Key::A,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? ftk::ChannelDisplay::Alpha : ftk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Alpha"]);

        addDivider();

        std::weak_ptr<App> appWeak(app);
        _actions["View/HUD"] = ftk::Action::create(
            "HUD",
            ftk::Key::H,
            static_cast<int>(ftk::commandKeyModifier),
            [appWeak](bool value)
            {
                if (auto app = appWeak.lock())
                {
                    auto options = app->getGlobalViewModel()->getOptions();
                    options.hud = value;
                    app->getGlobalViewModel()->setOptions(options);
                }
            });
        _actions["View/HUD"]->setTooltip("Toggle the HUD (Heads Up Display)");
        addAction(_actions["View/HUD"]);

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });

        _globalOptionsObserver = ftk::ValueObserver<GlobalViewOptions>::create(
            app->getGlobalViewModel()->observeOptions(),
            [this](const GlobalViewOptions& value)
            {
                setChecked(_actions["View/HUD"], value.hud);
            });
    }

    ViewMenu::~ViewMenu()
    {}

    std::shared_ptr<ViewMenu> ViewMenu::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& ViewMenu::getActions() const
    {
        return _actions;
    }

    void ViewMenu::_menuUpdate()
    {
        const bool file = _file.get();
        if (file)
        {
            _frameViewObserver = ftk::ValueObserver<bool>::create(
                _file->getViewModel()->observeFrameView(),
                [this](bool value)
                {
                    setChecked(_actions["View/Frame"], value);
                });

            _optionsObserver = ftk::ValueObserver<ViewOptions>::create(
                _file->getViewModel()->observeOptions(),
                [this](const ViewOptions& value)
                {
                    setChecked(_actions["View/Flip"], value.flip);
                    setChecked(_actions["View/Flop"], value.flop);
                    setChecked(_actions["View/Red"], ftk::ChannelDisplay::Red == value.channelDisplay);
                    setChecked(_actions["View/Green"], ftk::ChannelDisplay::Green == value.channelDisplay);
                    setChecked(_actions["View/Blue"], ftk::ChannelDisplay::Blue == value.channelDisplay);
                    setChecked(_actions["View/Alpha"], ftk::ChannelDisplay::Alpha == value.channelDisplay);
                });
        }
        else
        {
            setChecked(_actions["View/Frame"], false);
            setChecked(_actions["View/Flip"], false);
            setChecked(_actions["View/Flop"], false);
            _frameViewObserver.reset();
            _optionsObserver.reset();
        }

        setEnabled(_actions["View/ZoomIn"], file);
        setEnabled(_actions["View/ZoomOut"], file);
        setEnabled(_actions["View/ZoomReset"], file);
        setEnabled(_actions["View/Frame"], file);
        setEnabled(_actions["View/Flip"], file);
        setEnabled(_actions["View/Flop"], file);
        setEnabled(_actions["View/Red"], file);
        setEnabled(_actions["View/Green"], file);
        setEnabled(_actions["View/Blue"], file);
        setEnabled(_actions["View/Alpha"], file);
        setEnabled(_actions["View/HUD"], file);
    }
}
