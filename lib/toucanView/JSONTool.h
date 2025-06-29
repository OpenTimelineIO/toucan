// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>
#include <toucanView/SelectionModel.h>

#include <feather-tk/ui/Bellows.h>
#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/ui/SearchBox.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/ObservableList.h>

#include <opentimelineio/item.h>

namespace toucan
{
    class File;

    //! JSON widget.
    class JSONWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~JSONWidget();

        //! Create a new widget.
        static std::shared_ptr<JSONWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Set whether the widget is open.
        void setOpen(bool);

        //! Set the search.
        void setSearch(const std::string&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _textUpdate();

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata> _object;
        std::vector<std::string> _lineNumbers;
        std::vector<std::string> _text;
        std::string _search;
        std::shared_ptr<feather_tk::Bellows> _bellows;
        std::shared_ptr<feather_tk::Label> _lineNumbersLabel;
        std::shared_ptr<feather_tk::Label> _textLabel;
    };

    //! JSON tool.
    class JSONTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~JSONTool();

        //! Create a new tool.
        static std::shared_ptr<JSONTool> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<File> _file;

        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<feather_tk::VerticalLayout> _scrollLayout;
        std::vector<std::shared_ptr<JSONWidget> > _widgets;
        std::shared_ptr<feather_tk::HorizontalLayout> _bottomLayout;
        std::shared_ptr<feather_tk::SearchBox> _searchBox;
        std::shared_ptr<feather_tk::ToolButton> _openButton;
        std::shared_ptr<feather_tk::ToolButton> _closeButton;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<feather_tk::ListObserver<SelectionItem> > _selectionObserver;
    };
}

