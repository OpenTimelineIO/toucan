// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FileMenu.h"

#include "App.h"
#include "FilesModel.h"
#include "MainWindow.h"
#include "SelectionModel.h"

#include <toucanRender/Read.h>

#include <feather-tk/ui/Action.h>
#include <feather-tk/ui/FileBrowser.h>
#include <feather-tk/ui/RecentFilesModel.h>
#include <feather-tk/core/Format.h>

namespace toucan
{
    void FileMenu::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::Menu::_init(context, parent);

        _filesModel = app->getFilesModel();

        auto appWeak = std::weak_ptr<App>(app);
        auto windowWeak = std::weak_ptr<MainWindow>(window);
        _actions["File/Open"] = feather_tk::Action::create(
            "Open",
            "FileOpen",
            feather_tk::Key::O,
            static_cast<int>(feather_tk::commandKeyModifier),
            [this, appWeak, windowWeak]
            {
                auto context = getContext();
                auto fileBrowserSystem = context->getSystem<feather_tk::FileBrowserSystem>();
                if (_file)
                {
                    fileBrowserSystem->setPath(_file->getPath().parent_path());
                }

                std::vector<std::string> extensions;
                std::vector<std::string> extensionsTmp = ImageReadNode::getExtensions();
                extensions.insert(extensions.end(), extensionsTmp.begin(), extensionsTmp.end());
                extensionsTmp = SequenceReadNode::getExtensions();
                extensions.insert(extensions.end(), extensionsTmp.begin(), extensionsTmp.end());
                extensionsTmp = MovieReadNode::getExtensions();
                extensions.insert(extensions.end(), extensionsTmp.begin(), extensionsTmp.end());
                extensions.push_back(".otio");
                extensions.push_back(".otiod");
                extensions.push_back(".otioz");
                fileBrowserSystem->setExtensions(extensions);

                fileBrowserSystem->setRecentFilesModel(_filesModel->getRecentFilesModel());

                fileBrowserSystem->open(
                    windowWeak.lock(),
                    [appWeak](const std::filesystem::path& path)
                    {
                        appWeak.lock()->open(path);
                    });
            });
        _actions["File/Open"]->setTooltip("Open a file");
        addAction(_actions["File/Open"]);

        _actions["File/Close"] = feather_tk::Action::create(
            "Close",
            "FileClose",
            feather_tk::Key::E,
            static_cast<int>(feather_tk::commandKeyModifier),
            [this] { _filesModel->close(); });
        _actions["File/Close"]->setTooltip("Close the current file");
        addAction(_actions["File/Close"]);

        _actions["File/CloseAll"] = feather_tk::Action::create(
            "Close All",
            "FileCloseAll",
            feather_tk::Key::E,
            static_cast<int>(feather_tk::KeyModifier::Shift) | static_cast<int>(feather_tk::commandKeyModifier),
            [this] { _filesModel->closeAll(); });
        _actions["File/CloseAll"]->setTooltip("Close all files");
        addAction(_actions["File/CloseAll"]);

        addDivider();

        _menus["RecentFiles"] = addSubMenu("RecentFiles");

        addDivider();

        _menus["Files"] = addSubMenu("Files");

        _actions["File/Next"] = feather_tk::Action::create(
            "Next",
            feather_tk::Key::PageDown,
            0,
            [this] { _filesModel->next(); });
        _actions["File/Next"]->setTooltip("Switch to the next file");
        addAction(_actions["File/Next"]);

        _actions["File/Prev"] = feather_tk::Action::create(
            "Previous",
            feather_tk::Key::PageUp,
            0,
            [this] { _filesModel->prev(); });
        _actions["File/Prev"]->setTooltip("Switch to the previous file");
        addAction(_actions["File/Prev"]);

        addDivider();

        _actions["File/Exit"] = feather_tk::Action::create(
            "Exit",
            feather_tk::Key::Q,
            static_cast<int>(feather_tk::commandKeyModifier),
            [appWeak]
            {
                if (auto app = appWeak.lock())
                {
                    app->exit();
                }
            });
        addAction(_actions["File/Exit"]);

        _filesObserver = feather_tk::ListObserver<std::shared_ptr<File> >::create(
            _filesModel->observeFiles(),
            [this](const std::vector<std::shared_ptr<File> >& files)
            {
                _menus["Files"]->clear();
                _filesActions.clear();
                for (int i = 0; i < files.size(); ++i)
                {
                    auto action = feather_tk::Action::create(
                        files[i]->getPath().filename().string(),
                        [this, i]
                        {
                            _filesModel->setCurrentIndex(i);
                            close();
                        });
                    _menus["Files"]->addAction(action);
                    _filesActions.push_back(action);
                }
            });

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            _filesModel->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });

        _fileIndexObserver = feather_tk::ValueObserver<int>::create(
            _filesModel->observeCurrentIndex(),
            [this](int index)
            {
                for (int i = 0; i < _filesActions.size(); ++i)
                {
                    _menus["Files"]->setChecked(_filesActions[i], i == index);
                }
            });

        _recentFilesObserver = feather_tk::ListObserver<std::filesystem::path>::create(
            _filesModel->getRecentFilesModel()->observeRecent(),
            [this, appWeak](const std::vector<std::filesystem::path>& files)
            {
                _menus["RecentFiles"]->clear();
                _recentFilesActions.clear();
                for (auto i = files.rbegin(); i != files.rend(); ++i)
                {
                    auto file = *i;
                    auto action = feather_tk::Action::create(
                        file.string(),
                        [this, appWeak, file]
                        {
                            appWeak.lock()->open(file);
                            close();
                        });
                    _menus["RecentFiles"]->addAction(action);
                    _recentFilesActions.push_back(action);
                }
            });
    }

    FileMenu::~FileMenu()
    {}

    std::shared_ptr<FileMenu> FileMenu::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<FileMenu>(new FileMenu);
        out->_init(context, app, window, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<feather_tk::Action> >& FileMenu::getActions() const
    {
        return _actions;
    }

    void FileMenu::_menuUpdate()
    {
        const bool file = _file.get();
        setEnabled(_actions["File/Close"], file);
        setEnabled(_actions["File/CloseAll"], file);
        setSubMenuEnabled(_menus["Files"], file);
        setEnabled(_actions["File/Next"], _filesActions.size() > 1);
        setEnabled(_actions["File/Prev"], _filesActions.size() > 1);
    }
}
