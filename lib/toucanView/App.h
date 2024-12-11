// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <dtk/ui/App.h>

#include <filesystem>

namespace toucan
{
    class FilesModel;
    class ImageEffectHost;
    class MainWindow;
    class TimeUnitsModel;
    class WindowModel;

    //! Application.
    class App : public dtk::App
    {
    protected:
        void _init(
            const std::shared_ptr<dtk::Context>&,
            std::vector<std::string>&);

    public:
        virtual ~App();

        //! Create a new application.
        static std::shared_ptr<App> create(
            const std::shared_ptr<dtk::Context>&,
            std::vector<std::string>&);

        //! Get the time units model.
        const std::shared_ptr<TimeUnitsModel>& getTimeUnitsModel() const;

        //! Get the image effect host.
        const std::shared_ptr<ImageEffectHost>& getHost() const;

        //! Get the files model.
        const std::shared_ptr<FilesModel>& getFilesModel() const;

        //! Get the window model.
        const std::shared_ptr<WindowModel>& getWindowModel() const;

        //! Open a file.
        void open(const std::filesystem::path&);

    private:
        std::string _path;
        std::shared_ptr<TimeUnitsModel> _timeUnitsModel;
        std::shared_ptr<ImageEffectHost> _host;
        std::shared_ptr<FilesModel> _filesModel;
        std::shared_ptr<WindowModel> _windowModel;
        std::shared_ptr<MainWindow> _window;
    };
}

