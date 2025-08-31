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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::Menu::_init(context, parent);

        _filesModel = app->getFilesModel();

        auto appWeak = std::weak_ptr<App>(app);
        auto windowWeak = std::weak_ptr<MainWindow>(window);
        _actions["File/Open"] = ftk::Action::create(
            "Open",
            "FileOpen",
            ftk::Key::O,
            static_cast<int>(ftk::commandKeyModifier),
            [this, appWeak, windowWeak]
            {
                auto context = getContext();
                auto fileBrowserSystem = context->getSystem<ftk::FileBrowserSystem>();
                if (_file)
                {
                    fileBrowserSystem->getModel()->setPath(_file->getPath().parent_path());
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
                fileBrowserSystem->getModel()->setExtensions(extensions);

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

        _actions["File/Close"] = ftk::Action::create(
            "Close",
            "FileClose",
            ftk::Key::E,
            static_cast<int>(ftk::commandKeyModifier),
            [this] { _filesModel->close(); });
        _actions["File/Close"]->setTooltip("Close the current file");
        addAction(_actions["File/Close"]);

        _actions["File/CloseAll"] = ftk::Action::create(
            "Close All",
            "FileCloseAll",
            ftk::Key::E,
            static_cast<int>(ftk::KeyModifier::Shift) | static_cast<int>(ftk::commandKeyModifier),
            [this] { _filesModel->closeAll(); });
        _actions["File/CloseAll"]->setTooltip("Close all files");
        addAction(_actions["File/CloseAll"]);

        addDivider();

        _menus["RecentFiles"] = addSubMenu("RecentFiles");

        addDivider();

        _menus["Files"] = addSubMenu("Files");

        _actions["File/Next"] = ftk::Action::create(
            "Next",
            ftk::Key::PageDown,
            0,
            [this] { _filesModel->next(); });
        _actions["File/Next"]->setTooltip("Switch to the next file");
        addAction(_actions["File/Next"]);

        _actions["File/Prev"] = ftk::Action::create(
            "Previous",
            ftk::Key::PageUp,
            0,
            [this] { _filesModel->prev(); });
        _actions["File/Prev"]->setTooltip("Switch to the previous file");
        addAction(_actions["File/Prev"]);

        addDivider();

        _actions["File/Exit"] = ftk::Action::create(
            "Exit",
            ftk::Key::Q,
            static_cast<int>(ftk::commandKeyModifier),
            [appWeak]
            {
                if (auto app = appWeak.lock())
                {
                    app->exit();
                }
            });
        addAction(_actions["File/Exit"]);

        _filesObserver = ftk::ListObserver<std::shared_ptr<File> >::create(
            _filesModel->observeFiles(),
            [this](const std::vector<std::shared_ptr<File> >& files)
            {
                _menus["Files"]->clear();
                _filesActions.clear();
                for (int i = 0; i < files.size(); ++i)
                {
                    auto action = ftk::Action::create(
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

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            _filesModel->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });

        _fileIndexObserver = ftk::ValueObserver<int>::create(
            _filesModel->observeCurrentIndex(),
            [this](int index)
            {
                for (int i = 0; i < _filesActions.size(); ++i)
                {
                    _menus["Files"]->setChecked(_filesActions[i], i == index);
                }
            });

        _recentFilesObserver = ftk::ListObserver<std::filesystem::path>::create(
            _filesModel->getRecentFilesModel()->observeRecent(),
            [this, appWeak](const std::vector<std::filesystem::path>& files)
            {
                _menus["RecentFiles"]->clear();
                _recentFilesActions.clear();
                for (auto i = files.rbegin(); i != files.rend(); ++i)
                {
                    auto file = *i;
                    auto action = ftk::Action::create(
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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<FileMenu>(new FileMenu);
        out->_init(context, app, window, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& FileMenu::getActions() const
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
