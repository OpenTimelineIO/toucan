// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "SelectMenu.h"

#include "App.h"
#include "FilesModel.h"
#include "SelectionModel.h"

#include <feather-tk/ui/Action.h>

namespace toucan
{
    void SelectMenu::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::Menu::_init(context, parent);

        _actions["Select/All"] = feather_tk::Action::create(
            "All",
            feather_tk::Key::A,
            static_cast<int>(feather_tk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->selectAll(_file->getTimeline());
                }
            });
        addAction(_actions["Select/All"]);

        _actions["Select/AllTracks"] = feather_tk::Action::create(
            "All Tracks",
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->selectAll(
                        _file->getTimeline(),
                        SelectionType::Tracks);
                }
            });
        addAction(_actions["Select/AllTracks"]);

        _actions["Select/AllClips"] = feather_tk::Action::create(
            "All Clips",
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->selectAll(
                        _file->getTimeline(),
                        SelectionType::Clips);
                }
            });
        addAction(_actions["Select/AllClips"]);

        _actions["Select/AllMarkers"] = feather_tk::Action::create(
            "All Markers",
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->selectAll(
                        _file->getTimeline(),
                        SelectionType::Markers);
                }
            });
        addAction(_actions["Select/AllMarkers"]);

        _actions["Select/None"] = feather_tk::Action::create(
            "None",
            feather_tk::Key::A,
            static_cast<int>(feather_tk::KeyModifier::Shift) |
            static_cast<int>(feather_tk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->clearSelection();
                }
            });
        addAction(_actions["Select/None"]);

        _actions["Select/Invert"] = feather_tk::Action::create(
            "Invert",
            feather_tk::Key::I,
            static_cast<int>(feather_tk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->invertSelection(_file->getTimeline());
                }
            });
        addAction(_actions["Select/Invert"]);

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });
    }

    SelectMenu::~SelectMenu()
    {}

    std::shared_ptr<SelectMenu> SelectMenu::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<SelectMenu>(new SelectMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<feather_tk::Action> >& SelectMenu::getActions() const
    {
        return _actions;
    }

    void SelectMenu::_menuUpdate()
    {
        const bool file = _file.get();
        setEnabled(_actions["Select/All"], file);
        setEnabled(_actions["Select/AllTracks"], file);
        setEnabled(_actions["Select/AllClips"], file);
        setEnabled(_actions["Select/AllMarkers"], file);
        setEnabled(_actions["Select/None"], file);
        setEnabled(_actions["Select/Invert"], file);
    }
}
