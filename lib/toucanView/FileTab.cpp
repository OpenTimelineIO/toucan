// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FileTab.h"

#include "App.h"
#include "File.h"
#include "HUDWidget.h"
#include "Viewport.h"

namespace toucan
{
    void FileTab::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<IWidget>& parent)
    {
        ftk::IWidget::_init(context, "toucan::FileTab", parent);

        _viewport = Viewport::create(context, app, file, shared_from_this());

        _hudWidget = HUDWidget::create(context, app, file, shared_from_this());

        _viewOptionsObserver = ftk::ValueObserver<GlobalViewOptions>::create(
            app->getGlobalViewModel()->observeOptions(),
            [this](const GlobalViewOptions& value)
            {
                _hudWidget->setVisible(value.hud);
            });
    }

    FileTab::~FileTab()
    {}

    std::shared_ptr<FileTab> FileTab::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::shared_ptr<FileTab>(new FileTab);
        out->_init(context, app, file, parent);
        return out;
    }

    void FileTab::setGeometry(const ftk::Box2I& value)
    {
        ftk::IWidget::setGeometry(value);
        _viewport->setGeometry(value);
        _hudWidget->setGeometry(value);
    }

    void FileTab::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        ftk::IWidget::sizeHintEvent(event);
        _setSizeHint(_viewport->getSizeHint());
    }
}
