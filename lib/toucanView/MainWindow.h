// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/WindowModel.h>

#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/Splitter.h>
#include <feather-tk/ui/TabWidget.h>
#include <feather-tk/ui/Window.h>
#include <feather-tk/core/ObservableList.h>

namespace toucan
{
    class App;
    class File;
    class FileTab;
    class IToolWidget;
    class InfoBar;
    class MenuBar;
    class PlaybackBar;
    class TimelineWidget;
    class ToolBar;

    //! Main window.
    class MainWindow : public feather_tk::Window
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::string& name,
            const feather_tk::Size2I&);

    public:
        virtual ~MainWindow();

        //! Create a new window.
        static std::shared_ptr<MainWindow> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::string& name,
            const feather_tk::Size2I&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void keyPressEvent(feather_tk::KeyEvent&) override;
        void keyReleaseEvent(feather_tk::KeyEvent&) override;

    protected:
        void _drop(const std::vector<std::string>&) override;

    private:
        std::weak_ptr<App> _app;
        std::shared_ptr<feather_tk::Settings> _settings;
        std::vector<std::shared_ptr<File> > _files;

        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::shared_ptr<MenuBar> _menuBar;
        std::shared_ptr<ToolBar> _toolBar;
        std::shared_ptr<feather_tk::Divider> _toolBarDivider;
        std::shared_ptr<feather_tk::Splitter> _vSplitter;
        std::shared_ptr<feather_tk::Splitter> _hSplitter;
        std::shared_ptr<feather_tk::TabWidget> _tabWidget;
        std::map<std::shared_ptr<File>, std::shared_ptr<FileTab> > _fileTabs;
        std::shared_ptr<feather_tk::TabWidget> _toolWidget;
        std::vector<std::shared_ptr<IToolWidget> > _toolWidgets;
        std::shared_ptr<feather_tk::VerticalLayout> _playbackLayout;
        std::shared_ptr<PlaybackBar> _playbackBar;
        std::shared_ptr<TimelineWidget> _timelineWidget;
        std::shared_ptr<feather_tk::Divider> _infoBarDivider;
        std::shared_ptr<InfoBar> _infoBar;

        std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<File> > > _filesObserver;
        std::shared_ptr<feather_tk::ValueObserver<int> > _addObserver;
        std::shared_ptr<feather_tk::ValueObserver<int> > _removeObserver;
        std::shared_ptr<feather_tk::ValueObserver<int> > _fileObserver;
        std::shared_ptr<feather_tk::MapObserver<WindowComponent, bool> > _componentsObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _tooltipsObserver;
    };
}

