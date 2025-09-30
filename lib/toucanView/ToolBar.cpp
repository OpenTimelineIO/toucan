// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ToolBar.h"

#include "FileToolBar.h"
#include "ViewToolBar.h"
#include "WindowToolBar.h"

#include <ftk/Ui/Divider.h>

namespace toucan
{
    void ToolBar::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::IWidget::_init(context, "toucan::ToolBar", parent);

        _layout = ftk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        _toolBars["File"] = FileToolBar::create(context, app, actions, _layout);

        ftk::Divider::create(context, ftk::Orientation::Horizontal, _layout);

        _toolBars["View"] = ViewToolBar::create(context, app, actions, _layout);

        ftk::Divider::create(context, ftk::Orientation::Horizontal, _layout);

        _toolBars["Window"] = WindowToolBar::create(context, app, window, actions, _layout);
    }

    ToolBar::~ToolBar()
    {}

    std::shared_ptr<ToolBar> ToolBar::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ToolBar>(new ToolBar);
        out->_init(context, app, window, actions, parent);
        return out;
    }

    void ToolBar::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void ToolBar::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
