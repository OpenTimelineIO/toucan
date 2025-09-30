// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/WindowModel.h>

#include <ftk/Ui/Divider.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/Splitter.h>
#include <ftk/Ui/TabWidget.h>
#include <ftk/Ui/Window.h>
#include <ftk/Core/ObservableList.h>

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
    class MainWindow : public ftk::Window
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::string& name,
            const ftk::Size2I&);

    public:
        virtual ~MainWindow();

        //! Create a new window.
        static std::shared_ptr<MainWindow> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::string& name,
            const ftk::Size2I&);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void keyPressEvent(ftk::KeyEvent&) override;
        void keyReleaseEvent(ftk::KeyEvent&) override;

    protected:
        void _drop(const std::vector<std::string>&) override;

    private:
        std::weak_ptr<App> _app;
        std::shared_ptr<ftk::Settings> _settings;
        std::vector<std::shared_ptr<File> > _files;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<MenuBar> _menuBar;
        std::shared_ptr<ToolBar> _toolBar;
        std::shared_ptr<ftk::Divider> _toolBarDivider;
        std::shared_ptr<ftk::Splitter> _vSplitter;
        std::shared_ptr<ftk::Splitter> _hSplitter;
        std::shared_ptr<ftk::TabWidget> _tabWidget;
        std::map<std::shared_ptr<File>, std::shared_ptr<FileTab> > _fileTabs;
        std::shared_ptr<ftk::TabWidget> _toolWidget;
        std::vector<std::shared_ptr<IToolWidget> > _toolWidgets;
        std::shared_ptr<ftk::VerticalLayout> _playbackLayout;
        std::shared_ptr<PlaybackBar> _playbackBar;
        std::shared_ptr<TimelineWidget> _timelineWidget;
        std::shared_ptr<ftk::Divider> _infoBarDivider;
        std::shared_ptr<InfoBar> _infoBar;

        std::shared_ptr<ftk::ListObserver<std::shared_ptr<File> > > _filesObserver;
        std::shared_ptr<ftk::ValueObserver<int> > _addObserver;
        std::shared_ptr<ftk::ValueObserver<int> > _removeObserver;
        std::shared_ptr<ftk::ValueObserver<int> > _fileObserver;
        std::shared_ptr<ftk::MapObserver<WindowComponent, bool> > _componentsObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _tooltipsObserver;
    };
}

