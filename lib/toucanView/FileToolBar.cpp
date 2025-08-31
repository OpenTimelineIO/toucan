// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FileToolBar.h"

#include "App.h"
#include "FilesModel.h"

namespace toucan
{
    void FileToolBar::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::IWidget::_init(context, "toucan::FileToolBar", parent);

        _layout = ftk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        auto hLayout = ftk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
        std::vector<std::string> actionNames =
        {
            "File/Open",
            "File/Close",
            "File/CloseAll"
        };
        for (const auto& name : actionNames)
        {
            auto i = actions.find(name);
            auto button = ftk::ToolButton::create(context, i->second, hLayout);
            _buttons[name] = button;
        }

        _widgetUpdate();

        _filesObserver = ftk::ListObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeFiles(),
            [this](const std::vector<std::shared_ptr<File> >& files)
            {
                _filesSize = files.size();
                _widgetUpdate();
            });
    }

    FileToolBar::~FileToolBar()
    {}

    std::shared_ptr<FileToolBar> FileToolBar::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<FileToolBar>(new FileToolBar);
        out->_init(context, app, actions, parent);
        return out;
    }

    void FileToolBar::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void FileToolBar::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void FileToolBar::_widgetUpdate()
    {
        _buttons["File/Close"]->setEnabled(_filesSize > 0);
        _buttons["File/CloseAll"]->setEnabled(_filesSize > 0);
    }
}
