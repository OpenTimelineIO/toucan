// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "JSONTool.h"

#include "App.h"
#include "FilesModel.h"

#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/Spacer.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

namespace toucan
{
    void JSONWidget::_init(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>& object,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IWidget::_init(context, "toucan::JSONWidget", parent);

        _object = object;
        _text = ftk::split(object->to_json_string(), { '\n' });
        for (int i = 0; i < _text.size(); ++i)
        {
            _lineNumbers.push_back(ftk::Format("{0}").arg(i, ftk::digits(_text.size()), '0'));
        }

        _bellows = ftk::Bellows::create(context, object->name(), shared_from_this());
        _bellows->setOpen(true);

        auto hLayout = ftk::HorizontalLayout::create(context);
        hLayout->setSpacingRole(ftk::SizeRole::None);
        _bellows->setWidget(hLayout);

        _lineNumbersLabel = ftk::Label::create(context, hLayout);
        _lineNumbersLabel->setBackgroundRole(ftk::ColorRole::Base);
        _lineNumbersLabel->setFontRole(ftk::FontRole::Mono);
        _lineNumbersLabel->setMarginRole(ftk::SizeRole::MarginSmall);
        _lineNumbersLabel->setHStretch(ftk::Stretch::Fixed);

        _textLabel = ftk::Label::create(context, hLayout);
        _textLabel->setFontRole(ftk::FontRole::Mono);
        _textLabel->setMarginRole(ftk::SizeRole::MarginSmall);
        _textLabel->setHStretch(ftk::Stretch::Expanding);

        _textUpdate();
    }

    JSONWidget::~JSONWidget()
    {}

    std::shared_ptr<JSONWidget> JSONWidget::create(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>& object,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<JSONWidget>(new JSONWidget);
        out->_init(context, object, parent);
        return out;
    }

    void JSONWidget::setOpen(bool value)
    {
        _bellows->setOpen(value);
    }

    void JSONWidget::setSearch(const std::string& value)
    {
        if (value == _search)
            return;
        _search = value;
        _textUpdate();
    }

    void JSONWidget::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _bellows->setGeometry(value);
    }

    void JSONWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_bellows->getSizeHint());
    }

    void JSONWidget::_textUpdate()
    {
        if (!_search.empty())
        {
            std::vector<std::string> lineNumbers;
            std::vector<std::string> text;
            for (size_t i = 0; i < _lineNumbers.size() && i < _text.size(); ++i)
            {
                if (ftk::contains(_text[i], _search, ftk::CaseCompare::Insensitive))
                {
                    lineNumbers.push_back(_lineNumbers[i]);
                    text.push_back(_text[i]);
                }
            }
            _lineNumbersLabel->setText(ftk::join(lineNumbers, '\n'));
            _textLabel->setText(ftk::join(text, '\n'));
        }
        else
        {
            _lineNumbersLabel->setText(ftk::join(_lineNumbers, '\n'));
            _textLabel->setText(ftk::join(_text, '\n'));
        }
    }

    void JSONTool::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::JSONTool", "JSON", parent);

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::None);

        _scrollWidget = ftk::ScrollWidget::create(context, ftk::ScrollType::Both, _layout);
        _scrollWidget->setBorder(false);
        _scrollWidget->setVStretch(ftk::Stretch::Expanding);

        _scrollLayout = ftk::VerticalLayout::create(context);
        _scrollLayout->setSpacingRole(ftk::SizeRole::None);
        _scrollWidget->setWidget(_scrollLayout);

        ftk::Divider::create(context, ftk::Orientation::Vertical, _layout);

        _bottomLayout = ftk::HorizontalLayout::create(context, _layout);
        _bottomLayout->setMarginRole(ftk::SizeRole::MarginSmall);
        _bottomLayout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        _searchBox = ftk::SearchBox::create(context, _bottomLayout);
        _searchBox->setHStretch(ftk::Stretch::Expanding);
        _searchBox->setTooltip("Search the JSON text");

        auto hLayout = ftk::HorizontalLayout::create(context, _bottomLayout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
        _openButton = ftk::ToolButton::create(context, hLayout);
        _openButton->setIcon("BellowsOpen");
        _openButton->setTooltip("Open all");
        _closeButton = ftk::ToolButton::create(context, hLayout);
        _closeButton->setIcon("BellowsClosed");
        _closeButton->setTooltip("Close all");

        _searchBox->setCallback(
            [this](const std::string& text)
            {
                for (const auto& widget : _widgets)
                {
                    widget->setSearch(text);
                }
            });

        _openButton->setClickedCallback(
            [this]
            {
                for (const auto& widget : _widgets)
                {
                    widget->setOpen(true);
                }
            });

        _closeButton->setClickedCallback(
            [this]
            {
                for (const auto& widget : _widgets)
                {
                    widget->setOpen(false);
                }
            });

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                if (file)
                {
                    _selectionObserver = ftk::ListObserver<SelectionItem>::create(
                        file->getSelectionModel()->observeSelection(),
                        [this](const std::vector<SelectionItem>& selection)
                        {
                            for (const auto& widget : _widgets)
                            {
                                widget->setParent(nullptr);
                            }
                            _widgets.clear();
                            auto context = getContext();
                            const std::string& search = _searchBox->getText();
                            for (const auto& item : selection)
                            {
                                auto widget = JSONWidget::create(
                                    context,
                                    item.object,
                                    _scrollLayout);
                                widget->setSearch(search);
                                _widgets.push_back(widget);
                            }
                            _openButton->setEnabled(!_widgets.empty());
                            _closeButton->setEnabled(!_widgets.empty());
                        });
                }
                else
                {
                    for (const auto& widget : _widgets)
                    {
                        widget->setParent(nullptr);
                    }
                    _widgets.clear();
                    _openButton->setEnabled(false);
                    _closeButton->setEnabled(false);
                    _selectionObserver.reset();
                }
            });
    }

    JSONTool::~JSONTool()
    {}

    std::shared_ptr<JSONTool> JSONTool::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<JSONTool>(new JSONTool);
        out->_init(context, app, parent);
        return out;
    }

    void JSONTool::setGeometry(const ftk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void JSONTool::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
