// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>

#include <toucanRender/ImageGraph.h>

#include <feather-tk/ui/ButtonGroup.h>
#include <feather-tk/ui/PushButton.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>

namespace toucan
{
    class File;

    //! Image graph widget.
    class GraphWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~GraphWidget();

        //! Create a new widget.
        static std::shared_ptr<GraphWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

    private:
        int _getDepth(const std::shared_ptr<IImageNode>&, int = 0) const;

        void _createNodes(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<IImageNode>&,
            int = 0);

        void _drawInputs(
            const std::shared_ptr<IImageNode>&,
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent&,
            const feather_tk::LineOptions&);

        void _graphUpdate();

        std::shared_ptr<File> _file;
        std::shared_ptr<IImageNode> _rootNode;
        int _depth = 0;
        std::shared_ptr<IImageNode> _currentNode;

        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::vector<std::shared_ptr<feather_tk::HorizontalLayout> > _layouts;
        std::shared_ptr<feather_tk::ButtonGroup> _buttonGroup;
        std::vector<std::shared_ptr<feather_tk::PushButton> > _buttons;
        std::map<std::shared_ptr<IImageNode>, std::shared_ptr<feather_tk::PushButton> > _nodeToButton;
        std::map<std::shared_ptr<feather_tk::PushButton>, std::shared_ptr<IImageNode> > _buttonToNode;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int lineWidth = 0;
        };
        SizeData _size;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<IImageNode> > > _rootNodeObserver;
        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<IImageNode> > > _currentNodeObserver;
    };

    //! Image graph tool.
    class GraphTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~GraphTool();

        //! Create a new tool.
        static std::shared_ptr<GraphTool> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<GraphWidget> _widget;
    };
}

