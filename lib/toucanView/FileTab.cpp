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
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::FileTab", parent);

        _viewport = Viewport::create(context, app, file, shared_from_this());

        _hudWidget = HUDWidget::create(context, app, file, shared_from_this());

        _viewOptionsObserver = feather_tk::ValueObserver<GlobalViewOptions>::create(
            app->getGlobalViewModel()->observeOptions(),
            [this](const GlobalViewOptions& value)
            {
                _hudWidget->setVisible(value.hud);
            });
    }

    FileTab::~FileTab()
    {}

    std::shared_ptr<FileTab> FileTab::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::shared_ptr<FileTab>(new FileTab);
        out->_init(context, app, file, parent);
        return out;
    }

    void FileTab::setGeometry(const feather_tk::Box2I& value)
    {
        feather_tk::IWidget::setGeometry(value);
        _viewport->setGeometry(value);
        _hudWidget->setGeometry(value);
    }

    void FileTab::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        feather_tk::IWidget::sizeHintEvent(event);
        _setSizeHint(_viewport->getSizeHint());
    }
}
