// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "LogTool.h"

#include "App.h"

#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/Spacer.h>
#include <feather-tk/core/String.h>

namespace toucan
{
    namespace
    {
        const size_t textMax = 200;
    }

    void LogTool::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::LogTool", "Log", parent);

        _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::None);

        _scrollWidget = feather_tk::ScrollWidget::create(context, feather_tk::ScrollType::Both, _layout);
        _scrollWidget->setBorder(false);
        _scrollWidget->setVStretch(feather_tk::Stretch::Expanding);

        _label = feather_tk::Label::create(context);
        _label->setVAlign(feather_tk::VAlign::Top);
        _label->setFontRole(feather_tk::FontRole::Mono);
        _label->setMarginRole(feather_tk::SizeRole::MarginSmall);
        _scrollWidget->setWidget(_label);

        feather_tk::Divider::create(context, feather_tk::Orientation::Vertical, _layout);

        _bottomLayout = feather_tk::HorizontalLayout::create(context, _layout);
        _bottomLayout->setMarginRole(feather_tk::SizeRole::MarginSmall);
        _bottomLayout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        _searchBox = feather_tk::SearchBox::create(context, _bottomLayout);
        _searchBox->setHStretch(feather_tk::Stretch::Expanding);
        _searchBox->setTooltip("Search the log");

        _clearButton = feather_tk::ToolButton::create(context, _bottomLayout);
        _clearButton->setIcon("Clear");
        _clearButton->setTooltip("Clear the log");

        _textUpdate();

        _clearButton->setClickedCallback(
            [this]
            {
                _text.clear();
                _textUpdate();
            });

        _searchBox->setCallback(
            [this](const std::string&)
            {
                _textUpdate();
            });

        _logObserver = feather_tk::ListObserver<feather_tk::LogItem>::create(
            context->getSystem<feather_tk::LogSystem>()->observeLogItems(),
            [this](const std::vector<feather_tk::LogItem>& items)
            {
                for (const auto& item : items)
                {
                    for (const auto& line : feather_tk::split(feather_tk::toString(item), '\n'))
                    {
                        _text.push_back(line);
                    }
                }
                _textUpdate();
            });
    }

    LogTool::~LogTool()
    {}

    std::shared_ptr<LogTool> LogTool::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<LogTool>(new LogTool);
        out->_init(context, app, parent);
        return out;
    }

    void LogTool::setGeometry(const feather_tk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void LogTool::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void LogTool::_textUpdate()
    {
        while (_text.size() > textMax)
        {
            _text.pop_front();
        }

        std::vector<std::string> text;
        const std::string search = _searchBox->getText();
        if (search.empty())
        {
            text.insert(text.begin(), _text.begin(), _text.end());
        }
        else
        {
            for (const auto& line : _text)
            {
                if (feather_tk::contains(line, search, feather_tk::CaseCompare::Insensitive))
                {
                    text.push_back(line);
                }
            }
        }
        _label->setText(feather_tk::join(text, '\n'));

        _clearButton->setEnabled(!_text.empty());
    }
}
