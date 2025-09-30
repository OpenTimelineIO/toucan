// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>
#include <toucanView/SelectionModel.h>
#include <toucanView/TimeUnitsModel.h>

#include <toucanRender/ImageGraph.h>

#include <ftk/Ui/Bellows.h>
#include <ftk/Ui/ButtonGroup.h>
#include <ftk/Ui/GridLayout.h>
#include <ftk/Ui/Label.h>
#include <ftk/Ui/PushButton.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/ScrollWidget.h>
#include <ftk/Ui/SearchBox.h>
#include <ftk/Ui/ToolButton.h>
#include <ftk/Core/ObservableList.h>

#include <utility>

namespace toucan
{
    class File;

    //! Details widget.
    class DetailsWidget : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const SelectionItem&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~DetailsWidget();

        //! Create a new widget.
        static std::shared_ptr<DetailsWidget> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const SelectionItem&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set whether the widget is open.
        void setOpen(bool);

        //! Set the search.
        void setSearch(const std::string&);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        void _textUpdate();
        void _searchUpdate();

        std::weak_ptr<App> _app;
        std::shared_ptr<File> _file;
        TimeUnits _timeUnits = TimeUnits::First;
        SelectionItem _item;
        std::vector<std::pair<std::string, std::string> > _text;
        std::string _search;

        std::shared_ptr<ftk::Bellows> _bellows;
        std::shared_ptr<ftk::GridLayout> _layout;
        std::shared_ptr<ftk::ToolButton> _startFrameButton;
        std::shared_ptr<ftk::ToolButton> _inOutButton;
        std::vector<std::pair<std::shared_ptr<ftk::Label>, std::shared_ptr<ftk::Label> > > _labels;

        std::shared_ptr<ftk::ValueObserver<TimeUnits> > _timeUnitsObserver;
    };

    //! Details tool.
    class DetailsTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~DetailsTool();

        //! Create a new tool.
        static std::shared_ptr<DetailsTool> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<ftk::ScrollWidget> _scrollWidget;
        std::shared_ptr<ftk::VerticalLayout> _scrollLayout;
        std::vector<std::shared_ptr<DetailsWidget> > _widgets;
        std::shared_ptr<ftk::HorizontalLayout> _bottomLayout;
        std::shared_ptr<ftk::SearchBox> _searchBox;
        std::shared_ptr<ftk::ToolButton> _openButton;
        std::shared_ptr<ftk::ToolButton> _closeButton;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<ftk::ListObserver<SelectionItem> > _selectionObserver;
    };
}

