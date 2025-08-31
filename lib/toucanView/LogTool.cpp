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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::LogTool", "Log", parent);

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::None);

        _scrollWidget = ftk::ScrollWidget::create(context, ftk::ScrollType::Both, _layout);
        _scrollWidget->setBorder(false);
        _scrollWidget->setVStretch(ftk::Stretch::Expanding);

        _label = ftk::Label::create(context);
        _label->setVAlign(ftk::VAlign::Top);
        _label->setFontRole(ftk::FontRole::Mono);
        _label->setMarginRole(ftk::SizeRole::MarginSmall);
        _scrollWidget->setWidget(_label);

        ftk::Divider::create(context, ftk::Orientation::Vertical, _layout);

        _bottomLayout = ftk::HorizontalLayout::create(context, _layout);
        _bottomLayout->setMarginRole(ftk::SizeRole::MarginSmall);
        _bottomLayout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        _searchBox = ftk::SearchBox::create(context, _bottomLayout);
        _searchBox->setHStretch(ftk::Stretch::Expanding);
        _searchBox->setTooltip("Search the log");

        _clearButton = ftk::ToolButton::create(context, _bottomLayout);
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

        _logObserver = ftk::ListObserver<ftk::LogItem>::create(
            context->getSystem<ftk::LogSystem>()->observeLogItems(),
            [this](const std::vector<ftk::LogItem>& items)
            {
                for (const auto& item : items)
                {
                    for (const auto& line : ftk::split(ftk::toString(item), '\n'))
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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<LogTool>(new LogTool);
        out->_init(context, app, parent);
        return out;
    }

    void LogTool::setGeometry(const ftk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void LogTool::sizeHintEvent(const ftk::SizeHintEvent& event)
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
                if (ftk::contains(line, search, ftk::CaseCompare::Insensitive))
                {
                    text.push_back(line);
                }
            }
        }
        _label->setText(ftk::join(text, '\n'));

        _clearButton->setEnabled(!_text.empty());
    }
}
