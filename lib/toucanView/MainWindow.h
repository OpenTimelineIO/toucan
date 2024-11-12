// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "WindowModel.h"

#include <dtk/ui/Divider.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/Splitter.h>
#include <dtk/ui/TabWidget.h>
#include <dtk/ui/Window.h>
#include <dtk/core/ObservableList.h>

namespace toucan
{
    class App;
    class Document;
    class DocumentTab;
    class IToolWidget;
    class InfoBar;
    class MenuBar;
    class PlaybackBar;
    class TimelineWidget;
    class ToolBar;

    class MainWindow : public dtk::Window
    {
    protected:
        void _init(
            const std::shared_ptr<dtk::Context>&,
            const std::shared_ptr<App>&,
            const std::string& name,
            const dtk::Size2I&);

    public:
        virtual ~MainWindow();

        static std::shared_ptr<MainWindow> create(
            const std::shared_ptr<dtk::Context>&,
            const std::shared_ptr<App>&,
            const std::string& name,
            const dtk::Size2I&);

        void setGeometry(const dtk::Box2I&) override;
        void sizeHintEvent(const dtk::SizeHintEvent&) override;
        void keyPressEvent(dtk::KeyEvent&) override;
        void keyReleaseEvent(dtk::KeyEvent&) override;

    protected:
        void _drop(const std::vector<std::string>&) override;

    private:
        std::weak_ptr<App> _app;
        std::vector<std::shared_ptr<Document> > _documents;

        std::shared_ptr<dtk::VerticalLayout> _layout;
        std::shared_ptr<MenuBar> _menuBar;
        std::shared_ptr<ToolBar> _toolBar;
        std::shared_ptr<dtk::Divider> _toolBarDivider;
        std::shared_ptr<dtk::Splitter> _vSplitter;
        std::shared_ptr<dtk::Splitter> _hSplitter;
        std::shared_ptr<dtk::TabWidget> _tabWidget;
        std::map<std::shared_ptr<Document>, std::shared_ptr<DocumentTab> > _documentTabs;
        std::shared_ptr<dtk::TabWidget> _toolWidget;
        std::vector<std::shared_ptr<IToolWidget> > _toolWidgets;
        std::shared_ptr<dtk::VerticalLayout> _bottomLayout;
        std::shared_ptr<PlaybackBar> _playbackBar;
        std::shared_ptr<TimelineWidget> _timelineWidget;
        std::shared_ptr<dtk::Divider> _timelineDivider;
        std::shared_ptr<InfoBar> _infoBar;
        std::shared_ptr<dtk::Divider> _infoDivider;

        std::shared_ptr<dtk::ListObserver<std::shared_ptr<Document> > > _documentsObserver;
        std::shared_ptr<dtk::ValueObserver<int> > _addObserver;
        std::shared_ptr<dtk::ValueObserver<int> > _removeObserver;
        std::shared_ptr<dtk::ValueObserver<int> > _documentObserver;
        std::shared_ptr<dtk::MapObserver<WindowControl, bool> > _controlsObserver;
        std::shared_ptr<dtk::ValueObserver<bool> > _tooltipsObserver;
    };
}
