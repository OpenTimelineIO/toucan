// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>

#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/ui/SearchBox.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/LogSystem.h>

namespace toucan
{
    //! Log tool.
    class LogTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~LogTool();

        //! Create a new tool.
        static std::shared_ptr<LogTool> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        void _textUpdate();

        std::list<std::string> _text;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<ftk::ScrollWidget> _scrollWidget;
        std::shared_ptr<ftk::Label> _label;
        std::shared_ptr<ftk::HorizontalLayout> _bottomLayout;
        std::shared_ptr<ftk::SearchBox> _searchBox;
        std::shared_ptr<ftk::ToolButton> _clearButton;

        std::shared_ptr<ftk::ListObserver<ftk::LogItem> > _logObserver;
    };
}

