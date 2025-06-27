// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "CompareMenu.h"

#include "App.h"

#include <feather-tk/ui/Action.h>

namespace toucan
{
    void CompareMenu::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::Menu::_init(context, parent);

        _filesModel = app->getFilesModel();

        _menus["BFile"] = addSubMenu("B File");

        addDivider();

        _actions["Compare/A"] = feather_tk::Action::create(
            "A",
            feather_tk::Key::A,
            static_cast<int>(feather_tk::KeyModifier::Shift),
            [this](bool)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = CompareMode::A;
                _filesModel->setCompareOptions(options);
            });
        addAction(_actions["Compare/A"]);

        _actions["Compare/B"] = feather_tk::Action::create(
            "B",
            feather_tk::Key::B,
            static_cast<int>(feather_tk::KeyModifier::Shift),
            [this](bool)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = CompareMode::B;
                _filesModel->setCompareOptions(options);
            });
        addAction(_actions["Compare/B"]);

        _actions["Compare/Split"] = feather_tk::Action::create(
            "Split",
            feather_tk::Key::S,
            static_cast<int>(feather_tk::KeyModifier::Shift),
            [this](bool)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = CompareMode::Split;
                _filesModel->setCompareOptions(options);
            });
        addAction(_actions["Compare/Split"]);

        _actions["Compare/Overlay"] = feather_tk::Action::create(
            "Overlay",
            feather_tk::Key::E,
            static_cast<int>(feather_tk::KeyModifier::Shift),
            [this](bool)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = CompareMode::Overlay;
                _filesModel->setCompareOptions(options);
            });
        addAction(_actions["Compare/Overlay"]);

        _actions["Compare/Horizontal"] = feather_tk::Action::create(
            "Horizontal",
            feather_tk::Key::H,
            static_cast<int>(feather_tk::KeyModifier::Shift),
            [this](bool)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = CompareMode::Horizontal;
                _filesModel->setCompareOptions(options);
            });
        addAction(_actions["Compare/Horizontal"]);

        _actions["Compare/Vertical"] = feather_tk::Action::create(
            "Vertical",
            feather_tk::Key::V,
            static_cast<int>(feather_tk::KeyModifier::Shift),
            [this](bool)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = CompareMode::Vertical;
                _filesModel->setCompareOptions(options);
            });
        addAction(_actions["Compare/Vertical"]);

        _modeObserver = feather_tk::ValueObserver<CompareOptions>::create(
            app->getFilesModel()->observeCompareOptions(),
            [this](const CompareOptions& value)
            {
                setChecked(_actions["Compare/A"], CompareMode::A == value.mode);
                setChecked(_actions["Compare/B"], CompareMode::B == value.mode);
                setChecked(_actions["Compare/Split"], CompareMode::Split == value.mode);
                setChecked(_actions["Compare/Overlay"], CompareMode::Overlay == value.mode);
                setChecked(_actions["Compare/Horizontal"], CompareMode::Horizontal == value.mode);
                setChecked(_actions["Compare/Vertical"], CompareMode::Vertical == value.mode);
            });

        _filesObserver = feather_tk::ListObserver<std::shared_ptr<File> >::create(
            _filesModel->observeFiles(),
            [this](const std::vector<std::shared_ptr<File> >& files)
            {
                _menus["BFile"]->clear();
                _bFileActions.clear();
                for (int i = 0; i < files.size(); ++i)
                {
                    auto action = feather_tk::Action::create(
                        files[i]->getPath().filename().string(),
                        [this, i](bool value)
                        {
                            _filesModel->setBIndex(value ? i : -1);
                            close();
                        });
                    _menus["BFile"]->addAction(action);
                    _bFileActions.push_back(action);
                }
            });

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });

        _bIndexObserver = feather_tk::ValueObserver<int>::create(
            _filesModel->observeBIndex(),
            [this](int index)
            {
                for (int i = 0; i < _bFileActions.size(); ++i)
                {
                    _menus["BFile"]->setChecked(_bFileActions[i], i == index);
                }
            });
    }

    CompareMenu::~CompareMenu()
    {}

    std::shared_ptr<CompareMenu> CompareMenu::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<feather_tk::Action> >& CompareMenu::getActions() const
    {
        return _actions;
    }

    void CompareMenu::_menuUpdate()
    {
        const bool file = _file.get();
        setSubMenuEnabled(_menus["BFile"], file);
        setEnabled(_actions["Compare/A"], file);
        setEnabled(_actions["Compare/B"], file);
        setEnabled(_actions["Compare/Split"], file);
        setEnabled(_actions["Compare/Overlay"], file);
        setEnabled(_actions["Compare/Horizontal"], file);
        setEnabled(_actions["Compare/Vertical"], file);
    }
}
