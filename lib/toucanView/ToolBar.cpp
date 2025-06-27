// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ToolBar.h"

#include "FileToolBar.h"
#include "ViewToolBar.h"
#include "WindowToolBar.h"

#include <feather-tk/ui/Divider.h>

namespace toucan
{
    void ToolBar::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::ToolBar", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        _toolBars["File"] = FileToolBar::create(context, app, actions, _layout);

        feather_tk::Divider::create(context, feather_tk::Orientation::Horizontal, _layout);

        _toolBars["View"] = ViewToolBar::create(context, app, actions, _layout);

        feather_tk::Divider::create(context, feather_tk::Orientation::Horizontal, _layout);

        _toolBars["Window"] = WindowToolBar::create(context, app, window, actions, _layout);
    }

    ToolBar::~ToolBar()
    {}

    std::shared_ptr<ToolBar> ToolBar::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<MainWindow>& window,
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ToolBar>(new ToolBar);
        out->_init(context, app, window, actions, parent);
        return out;
    }

    void ToolBar::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void ToolBar::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
