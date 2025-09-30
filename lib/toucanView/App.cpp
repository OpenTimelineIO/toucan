// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "App.h"

#include "FilesModel.h"
#include "MainWindow.h"
#include "TimeUnitsModel.h"
#include "ViewModel.h"
#include "WindowModel.h"

#include <toucanRender/Util.h>

#include <ftk/Ui/DialogSystem.h>
#include <ftk/Ui/FileBrowser.h>
#include <ftk/Ui/MessageDialog.h>
#include <ftk/Ui/Settings.h>

#include <ftk/Core/CmdLine.h>

#include <nlohmann/json.hpp>

namespace toucan
{
    void App::_init(
        const std::shared_ptr<ftk::Context>& context,
        std::vector<std::string>& argv)
    {
        _input = ftk::CmdLineValueArg<std::string>::create(
            "input",
            "Input timeline.",
            true);

        ftk::App::_init(
            context,
            argv,
            "toucan-view",
            "Toucan viewer",
            { _input });

        _settings = ftk::Settings::create(context, ftk::getSettingsPath("toucan", "toucan-view.settings"));

        _timeUnitsModel = std::make_shared<TimeUnitsModel>(context, _settings);

        _host = std::make_shared<ImageEffectHost>(context, getOpenFXPluginPaths(getExeName()));

        auto fileBrowserSystem = context->getSystem<ftk::FileBrowserSystem>();
        fileBrowserSystem->setNativeFileDialog(false);

        _filesModel = std::make_shared<FilesModel>(context, _settings, _host);
        _globalViewModel = std::make_shared<GlobalViewModel>(context, _settings);
        _windowModel = std::make_shared<WindowModel>(context, _settings);

        _window = MainWindow::create(
            context,
            std::dynamic_pointer_cast<App>(shared_from_this()),
            "toucan-view",
            ftk::Size2I(1920, 1080));
        addWindow(_window);
        _window->show();

        if (_input->hasValue())
        {
            open(_input->getValue());
        }
    }

    App::~App()
    {}

    std::shared_ptr<App> App::create(
        const std::shared_ptr<ftk::Context>& context,
        std::vector<std::string>& argv)
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(context, argv);
        return out;
    }

    const std::shared_ptr<ftk::Settings>& App::getSettings() const
    {
        return _settings;
    }

    const std::shared_ptr<TimeUnitsModel>& App::getTimeUnitsModel() const
    {
        return _timeUnitsModel;
    }

    const std::shared_ptr<ImageEffectHost>& App::getHost() const
    {
        return _host;
    }

    const std::shared_ptr<FilesModel>& App::getFilesModel() const
    {
        return _filesModel;
    }

    const std::shared_ptr<GlobalViewModel>& App::getGlobalViewModel() const
    {
        return _globalViewModel;
    }

    const std::shared_ptr<WindowModel>& App::getWindowModel() const
    {
        return _windowModel;
    }

    void App::open(const std::filesystem::path& path)
    {
        try
        {
            _filesModel->open(path);
        }
        catch (const std::exception& e)
        {
            _context->getSystem<ftk::DialogSystem>()->message(
                "ERROR",
                e.what(),
                _window);
        }
    }
}
