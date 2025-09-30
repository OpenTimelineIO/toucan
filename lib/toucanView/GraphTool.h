// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>

#include <toucanRender/ImageGraph.h>

#include <ftk/Ui/ButtonGroup.h>
#include <ftk/Ui/PushButton.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/ScrollWidget.h>

namespace toucan
{
    class File;

    //! Image graph widget.
    class GraphWidget : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~GraphWidget();

        //! Create a new widget.
        static std::shared_ptr<GraphWidget> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

    private:
        int _getDepth(const std::shared_ptr<IImageNode>&, int = 0) const;

        void _createNodes(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<IImageNode>&,
            int = 0);

        void _drawInputs(
            const std::shared_ptr<IImageNode>&,
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent&,
            const ftk::LineOptions&);

        void _graphUpdate();

        std::shared_ptr<File> _file;
        std::shared_ptr<IImageNode> _rootNode;
        int _depth = 0;
        std::shared_ptr<IImageNode> _currentNode;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::vector<std::shared_ptr<ftk::HorizontalLayout> > _layouts;
        std::shared_ptr<ftk::ButtonGroup> _buttonGroup;
        std::vector<std::shared_ptr<ftk::PushButton> > _buttons;
        std::map<std::shared_ptr<IImageNode>, std::shared_ptr<ftk::PushButton> > _nodeToButton;
        std::map<std::shared_ptr<ftk::PushButton>, std::shared_ptr<IImageNode> > _buttonToNode;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int lineWidth = 0;
        };
        SizeData _size;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<IImageNode> > > _rootNodeObserver;
        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<IImageNode> > > _currentNodeObserver;
    };

    //! Image graph tool.
    class GraphTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~GraphTool();

        //! Create a new tool.
        static std::shared_ptr<GraphTool> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        std::shared_ptr<ftk::ScrollWidget> _scrollWidget;
        std::shared_ptr<GraphWidget> _widget;
    };
}

