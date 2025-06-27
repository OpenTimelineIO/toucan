// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ViewToolBar.h"

#include "App.h"
#include "FilesModel.h"
#include "ViewModel.h"

namespace toucan
{
    void ViewToolBar::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::ViewToolBar", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        auto hLayout = feather_tk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(feather_tk::SizeRole::SpacingTool);
        std::vector<std::string> actionNames =
        {
            "View/ZoomIn",
            "View/ZoomOut",
            "View/ZoomReset"
        };
        for (const auto& name : actionNames)
        {
            auto i = actions.find(name);
            auto button = feather_tk::ToolButton::create(context, i->second, hLayout);
            _buttons[name] = button;
        }
        
        auto i = actions.find("View/Frame");
        auto button = feather_tk::ToolButton::create(context, i->second, hLayout);
        _buttons["View/Frame"] = button;

        _widgetUpdate();

        _filesObserver = feather_tk::ListObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeFiles(),
            [this](const std::vector<std::shared_ptr<File> >& files)
            {
                _filesSize = files.size();
                _widgetUpdate();
            });

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _widgetUpdate();
            });
    }

    ViewToolBar::~ViewToolBar()
    {}

    std::shared_ptr<ViewToolBar> ViewToolBar::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ViewToolBar>(new ViewToolBar);
        out->_init(context, app, actions, parent);
        return out;
    }

    void ViewToolBar::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void ViewToolBar::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void ViewToolBar::_widgetUpdate()
    {
        _buttons["View/ZoomIn"]->setEnabled(_filesSize > 0);
        _buttons["View/ZoomOut"]->setEnabled(_filesSize > 0);
        _buttons["View/ZoomReset"]->setEnabled(_filesSize > 0);
        _buttons["View/Frame"]->setEnabled(_filesSize > 0);

        if (_file)
        {
            _frameViewObserver = feather_tk::ValueObserver<bool>::create(
                _file->getViewModel()->observeFrameView(),
                [this](bool value)
                {
                    _buttons["View/Frame"]->setChecked(value);
                });
        }
        else
        {
            _frameViewObserver.reset();
        }
    }
}
