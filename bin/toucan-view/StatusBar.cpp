// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "StatusBar.h"

#include "App.h"

#include <dtk/core/String.h>

namespace toucan
{
    void StatusBar::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        dtk::IWidget::_init(context, "toucan::StatusBar", parent);

        _layout = dtk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(dtk::SizeRole::SpacingTool);

        _messageLabel = dtk::Label::create(context, _layout);
        _messageLabel->setHStretch(dtk::Stretch::Expanding);
        _messageLabel->setMarginRole(dtk::SizeRole::MarginInside);
    }

    StatusBar::~StatusBar()
    {}

    std::shared_ptr<StatusBar> StatusBar::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        auto out = std::shared_ptr<StatusBar>(new StatusBar);
        out->_init(context, app, parent);
        return out;
    }

    void StatusBar::setGeometry(const dtk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void StatusBar::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}