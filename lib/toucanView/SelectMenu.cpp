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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::Menu::_init(context, parent);

        _actions["Select/All"] = ftk::Action::create(
            "All",
            ftk::Key::A,
            static_cast<int>(ftk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->selectAll(_file->getTimeline());
                }
            });
        addAction(_actions["Select/All"]);

        _actions["Select/AllTracks"] = ftk::Action::create(
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

        _actions["Select/AllClips"] = ftk::Action::create(
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

        _actions["Select/AllMarkers"] = ftk::Action::create(
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

        _actions["Select/None"] = ftk::Action::create(
            "None",
            ftk::Key::A,
            static_cast<int>(ftk::KeyModifier::Shift) |
            static_cast<int>(ftk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->clearSelection();
                }
            });
        addAction(_actions["Select/None"]);

        _actions["Select/Invert"] = ftk::Action::create(
            "Invert",
            ftk::Key::I,
            static_cast<int>(ftk::commandKeyModifier),
            [this]
            {
                if (_file)
                {
                    _file->getSelectionModel()->invertSelection(_file->getTimeline());
                }
            });
        addAction(_actions["Select/Invert"]);

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<SelectMenu>(new SelectMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& SelectMenu::getActions() const
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
