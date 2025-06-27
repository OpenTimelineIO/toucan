// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "GraphTool.h"

#include "App.h"
#include "FilesModel.h"

#include <feather-tk/ui/Spacer.h>

namespace toucan
{
    void GraphWidget::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IWidget::_init(context, "toucan::GraphWidget", parent);

        _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
        _layout->setMarginRole(feather_tk::SizeRole::MarginLarge);
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingLarge);

        _buttonGroup = feather_tk::ButtonGroup::create(context, feather_tk::ButtonGroupType::Radio);

        _buttonGroup->setCheckedCallback(
            [this](int index, bool value)
            {
                if (_file && index >= 0 && index < _buttons.size())
                {
                    auto i = _buttonToNode.find(_buttons[index]);
                    if (i != _buttonToNode.end())
                    {
                        _file->setCurrentNode(i->second);
                    }
                }
            });

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                if (file)
                {
                    _rootNodeObserver = feather_tk::ValueObserver<std::shared_ptr<IImageNode> >::create(
                        file->observeRootNode(),
                        [this](const std::shared_ptr<IImageNode>& node)
                        {
                            _rootNode = node;
                            _depth = _getDepth(_rootNode);
                            _graphUpdate();
                        });

                    _currentNodeObserver = feather_tk::ValueObserver<std::shared_ptr<IImageNode> >::create(
                        file->observeCurrentNode(),
                        [this](const std::shared_ptr<IImageNode>& node)
                        {
                            _currentNode = node;
                            auto i = _nodeToButton.find(node);
                            if (i != _nodeToButton.end())
                            {
                                i->second->setChecked(true);
                            }
                        });
                }
                else
                {
                    _rootNode.reset();
                    _depth = 0;
                    _currentNode.reset();
                    _rootNodeObserver.reset();
                    _currentNodeObserver.reset();
                    _graphUpdate();
                }
            });
    }

    GraphWidget::~GraphWidget()
    {}

    std::shared_ptr<GraphWidget> GraphWidget::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<GraphWidget>(new GraphWidget);
        out->_init(context, app, parent);
        return out;
    }

    void GraphWidget::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void GraphWidget::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.lineWidth = 2 * event.displayScale;
        }
        _setSizeHint(_layout->getSizeHint());
    }

    void GraphWidget::drawEvent(const feather_tk::Box2I& drawRect, const feather_tk::DrawEvent& event)
    {
        IWidget::drawEvent(drawRect, event);
        if (_rootNode)
        {
            feather_tk::LineOptions options;
            options.width = _size.lineWidth;
            _drawInputs(_rootNode, drawRect, event, options);
        }
    }

    int GraphWidget::_getDepth(const std::shared_ptr<IImageNode>& node, int depth) const
    {
        int out = depth + 1;
        for (const auto& input : node->getInputs())
        {
            if (input)
            {
                out = std::max(out, _getDepth(input, out));
            }
        }
        return out;
    }

    void GraphWidget::_createNodes(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<IImageNode>& node,
        int depth)
    {
        if (node)
        {
            auto button = feather_tk::PushButton::create(context, node->getLabel(), _layouts[_depth - 1 - depth]);
            _buttonGroup->addButton(button);
            _buttons.push_back(button);
            _nodeToButton[node] = button;
            _buttonToNode[button] = node;
            for (const auto& input : node->getInputs())
            {
                _createNodes(context, input, depth + 1);
            }
        }
    }

    void GraphWidget::_drawInputs(
        const std::shared_ptr<IImageNode>& node,
        const feather_tk::Box2I& drawRect,
        const feather_tk::DrawEvent& event,
        const feather_tk::LineOptions& options)
    {
        auto i = _nodeToButton.find(node);
        if (i != _nodeToButton.end())
        {
            feather_tk::Box2I g = i->second->getGeometry();
            const feather_tk::V2I start = feather_tk::center(g);
            std::vector<std::pair<feather_tk::V2I, feather_tk::V2I> > lines;
            for (const auto& input : node->getInputs())
            {
                auto j = _nodeToButton.find(input);
                if (j != _nodeToButton.end())
                {
                    g = j->second->getGeometry();
                    const feather_tk::V2I end = feather_tk::center(g);
                    const feather_tk::V2I v0(start.x, (start.y + end.y) / 2);
                    const feather_tk::V2I v1(end.x, (start.y + end.y) / 2);
                    lines.push_back(std::make_pair(start, v0));
                    lines.push_back(std::make_pair(v0, v1));
                    lines.push_back(std::make_pair(v1, end));
                }
            }
            event.render->drawLines(
                lines,
                event.style->getColorRole(feather_tk::ColorRole::Checked),
                options);
            for (const auto& input : node->getInputs())
            {
                _drawInputs(input, drawRect, event, options);
            }
        }
    }

    void GraphWidget::_graphUpdate()
    {
        _buttonGroup->clearButtons();
        _buttons.clear();
        _nodeToButton.clear();
        _buttonToNode.clear();
        for (const auto& layout : _layouts)
        {
            layout->setParent(nullptr);
        }
        _layouts.clear();
        if (_rootNode)
        {
            auto context = getContext();
            for (int i = 0; i < _depth; ++i)
            {
                auto layout = feather_tk::HorizontalLayout::create(context, _layout);
                auto spacer = feather_tk::Spacer::create(context, feather_tk::Orientation::Horizontal, layout);
                spacer->setSpacingRole(feather_tk::SizeRole::None);
                spacer->setStretch(feather_tk::Stretch::Expanding);
                _layouts.push_back(layout);
            }
            _createNodes(context, _rootNode);
            for (const auto& layout : _layouts)
            {
                auto spacer = feather_tk::Spacer::create(context, feather_tk::Orientation::Horizontal, layout);
                spacer->setSpacingRole(feather_tk::SizeRole::None);
                spacer->setStretch(feather_tk::Stretch::Expanding);
            }
        }
    }

    void GraphTool::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::GraphTool", "Graph", parent);

        _scrollWidget = feather_tk::ScrollWidget::create(context, feather_tk::ScrollType::Both, shared_from_this());
        _scrollWidget->setBorder(false);

        _widget = GraphWidget::create(context, app);
        _scrollWidget->setWidget(_widget);
    }

    GraphTool::~GraphTool()
    {}

    std::shared_ptr<GraphTool> GraphTool::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<GraphTool>(new GraphTool);
        out->_init(context, app, parent);
        return out;
    }

    void GraphTool::setGeometry(const feather_tk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _scrollWidget->setGeometry(value);
    }

    void GraphTool::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_scrollWidget->getSizeHint());
    }
}
