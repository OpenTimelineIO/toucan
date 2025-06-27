// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ViewMenu.h"

#include "App.h"
#include "FilesModel.h"

namespace toucan
{
    void ViewMenu::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::Menu::_init(context, parent);

        _actions["View/ZoomIn"] = feather_tk::Action::create(
            "Zoom In",
            "ViewZoomIn",
            feather_tk::Key::Equal,
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

        _actions["View/ZoomOut"] = feather_tk::Action::create(
            "Zoom Out",
            "ViewZoomOut",
            feather_tk::Key::Minus,
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

        _actions["View/ZoomReset"] = feather_tk::Action::create(
            "Zoom Reset",
            "ViewZoomReset",
            feather_tk::Key::_0,
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

        _actions["View/Frame"] = feather_tk::Action::create(
            "Frame View",
            "ViewFrame",
            feather_tk::Key::Backspace,
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

        _actions["View/Flip"] = feather_tk::Action::create(
            "Flip Vertical",
            feather_tk::Key::V,
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

        _actions["View/Flop"] = feather_tk::Action::create(
            "Flop Horizontal",
            feather_tk::Key::H,
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

        _actions["View/Red"] = feather_tk::Action::create(
            "Red",
            feather_tk::Key::R,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? feather_tk::ChannelDisplay::Red : feather_tk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Red"]);

        _actions["View/Green"] = feather_tk::Action::create(
            "Green",
            feather_tk::Key::G,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? feather_tk::ChannelDisplay::Green : feather_tk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Green"]);

        _actions["View/Blue"] = feather_tk::Action::create(
            "Blue",
            feather_tk::Key::B,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? feather_tk::ChannelDisplay::Blue : feather_tk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Blue"]);

        _actions["View/Alpha"] = feather_tk::Action::create(
            "Alpha",
            feather_tk::Key::A,
            0,
            [this](bool value)
            {
                if (_file)
                {
                    ViewOptions options = _file->getViewModel()->getOptions();
                    options.channelDisplay = value ? feather_tk::ChannelDisplay::Alpha : feather_tk::ChannelDisplay::Color;
                    _file->getViewModel()->setOptions(options);
                }
            });
        addAction(_actions["View/Alpha"]);

        addDivider();

        std::weak_ptr<App> appWeak(app);
        _actions["View/HUD"] = feather_tk::Action::create(
            "HUD",
            feather_tk::Key::H,
            static_cast<int>(feather_tk::commandKeyModifier),
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

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });

        _globalOptionsObserver = feather_tk::ValueObserver<GlobalViewOptions>::create(
            app->getGlobalViewModel()->observeOptions(),
            [this](const GlobalViewOptions& value)
            {
                setChecked(_actions["View/HUD"], value.hud);
            });
    }

    ViewMenu::~ViewMenu()
    {}

    std::shared_ptr<ViewMenu> ViewMenu::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<feather_tk::Action> >& ViewMenu::getActions() const
    {
        return _actions;
    }

    void ViewMenu::_menuUpdate()
    {
        const bool file = _file.get();
        if (file)
        {
            _frameViewObserver = feather_tk::ValueObserver<bool>::create(
                _file->getViewModel()->observeFrameView(),
                [this](bool value)
                {
                    setChecked(_actions["View/Frame"], value);
                });

            _optionsObserver = feather_tk::ValueObserver<ViewOptions>::create(
                _file->getViewModel()->observeOptions(),
                [this](const ViewOptions& value)
                {
                    setChecked(_actions["View/Flip"], value.flip);
                    setChecked(_actions["View/Flop"], value.flop);
                    setChecked(_actions["View/Red"], feather_tk::ChannelDisplay::Red == value.channelDisplay);
                    setChecked(_actions["View/Green"], feather_tk::ChannelDisplay::Green == value.channelDisplay);
                    setChecked(_actions["View/Blue"], feather_tk::ChannelDisplay::Blue == value.channelDisplay);
                    setChecked(_actions["View/Alpha"], feather_tk::ChannelDisplay::Alpha == value.channelDisplay);
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
