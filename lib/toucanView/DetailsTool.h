// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>
#include <toucanView/SelectionModel.h>
#include <toucanView/TimeUnitsModel.h>

#include <toucanRender/ImageGraph.h>

#include <feather-tk/ui/Bellows.h>
#include <feather-tk/ui/ButtonGroup.h>
#include <feather-tk/ui/GridLayout.h>
#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/PushButton.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/ui/SearchBox.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/ObservableList.h>

#include <utility>

namespace toucan
{
    class File;

    //! Details widget.
    class DetailsWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const SelectionItem&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~DetailsWidget();

        //! Create a new widget.
        static std::shared_ptr<DetailsWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const SelectionItem&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set whether the widget is open.
        void setOpen(bool);

        //! Set the search.
        void setSearch(const std::string&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _textUpdate();
        void _searchUpdate();

        std::weak_ptr<App> _app;
        std::shared_ptr<File> _file;
        TimeUnits _timeUnits = TimeUnits::First;
        SelectionItem _item;
        std::vector<std::pair<std::string, std::string> > _text;
        std::string _search;

        std::shared_ptr<feather_tk::Bellows> _bellows;
        std::shared_ptr<feather_tk::GridLayout> _layout;
        std::shared_ptr<feather_tk::ToolButton> _startFrameButton;
        std::shared_ptr<feather_tk::ToolButton> _inOutButton;
        std::vector<std::pair<std::shared_ptr<feather_tk::Label>, std::shared_ptr<feather_tk::Label> > > _labels;

        std::shared_ptr<feather_tk::ValueObserver<TimeUnits> > _timeUnitsObserver;
    };

    //! Details tool.
    class DetailsTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~DetailsTool();

        //! Create a new tool.
        static std::shared_ptr<DetailsTool> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<feather_tk::VerticalLayout> _scrollLayout;
        std::vector<std::shared_ptr<DetailsWidget> > _widgets;
        std::shared_ptr<feather_tk::HorizontalLayout> _bottomLayout;
        std::shared_ptr<feather_tk::SearchBox> _searchBox;
        std::shared_ptr<feather_tk::ToolButton> _openButton;
        std::shared_ptr<feather_tk::ToolButton> _closeButton;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<feather_tk::ListObserver<SelectionItem> > _selectionObserver;
    };
}

