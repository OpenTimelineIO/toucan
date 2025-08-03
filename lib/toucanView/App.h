// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/App.h>
#include <feather-tk/ui/Settings.h>

#include <filesystem>

namespace toucan
{
    class FilesModel;
    class GlobalViewModel;
    class ImageEffectHost;
    class MainWindow;
    class TimeUnitsModel;
    class WindowModel;

    //! Application.
    class App : public feather_tk::App
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            std::vector<std::string>&);

    public:
        virtual ~App();

        //! Create a new application.
        static std::shared_ptr<App> create(
            const std::shared_ptr<feather_tk::Context>&,
            std::vector<std::string>&);

        //! Get the settings.
        const std::shared_ptr<feather_tk::Settings>& getSettings() const;

        //! Get the time units model.
        const std::shared_ptr<TimeUnitsModel>& getTimeUnitsModel() const;

        //! Get the image effect host.
        const std::shared_ptr<ImageEffectHost>& getHost() const;

        //! Get the files model.
        const std::shared_ptr<FilesModel>& getFilesModel() const;

        //! Get the global view model.
        const std::shared_ptr<GlobalViewModel>& getGlobalViewModel() const;

        //! Get the window model.
        const std::shared_ptr<WindowModel>& getWindowModel() const;

        //! Open a file.
        void open(const std::filesystem::path&);

    private:
        std::shared_ptr<feather_tk::CmdLineValueArg<std::string> > _input;
        std::shared_ptr<feather_tk::Settings> _settings;
        std::shared_ptr<TimeUnitsModel> _timeUnitsModel;
        std::shared_ptr<ImageEffectHost> _host;
        std::shared_ptr<FilesModel> _filesModel;
        std::shared_ptr<GlobalViewModel> _globalViewModel;
        std::shared_ptr<WindowModel> _windowModel;
        std::shared_ptr<MainWindow> _window;
    };
}

