// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "Window.h"

#include "App.h"
#include "DocumentTab.h"
#include "DocumentsModel.h"
#include "MenuBar.h"

#include <dtk/ui/Divider.h>
#include <dtk/ui/MessageDialog.h>
#include <dtk/core/String.h>

namespace toucan
{
    void Window::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::string& name,
        const dtk::Size2I& size)
    {
        dtk::Window::_init(context, name, size);

        _layout = dtk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(dtk::SizeRole::None);

        _menuBar = MenuBar::create(
            context,
            app,
            std::dynamic_pointer_cast<Window>(shared_from_this()),
            _layout);
        dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);

        _tabWidget = dtk::TabWidget::create(context, _layout);
        _tabWidget->setTabsClosable(true);
        _tabWidget->setVStretch(dtk::Stretch::Expanding);

        _app = app;

        std::weak_ptr<App> appWeak(app);
        _tabWidget->setCallback(
            [appWeak](int index)
            {
                if (auto app = appWeak.lock())
                {
                    app->getDocumentsModel()->setCurrentIndex(index);
                }
            });
        _tabWidget->setTabCloseCallback(
            [appWeak](int index)
            {
                if (auto app = appWeak.lock())
                {
                    app->getDocumentsModel()->close(index);
                }
            });

        _documentsObserver = dtk::ListObserver<std::shared_ptr<Document> >::create(
            app->getDocumentsModel()->observeDocuments(),
            [this](const std::vector<std::shared_ptr<Document> >& documents)
            {
                _documents = documents;
            });

        _addObserver = dtk::ValueObserver<int>::create(
            app->getDocumentsModel()->observeAdd(),
            [this, appWeak](int index)
            {
                if (index >= 0 && index < _documents.size())
                {
                    auto document = _documents[index];
                    auto context = _getContext().lock();
                    auto app = appWeak.lock();
                    auto tab = DocumentTab::create(context, app, document);
                    _tabWidget->addTab(
                        dtk::elide(document->getPath().filename().string()),
                        tab,
                        document->getPath().string());
                    _documentTabs[document] = tab;
                }
            });

        _removeObserver = dtk::ValueObserver<int>::create(
            app->getDocumentsModel()->observeRemove(),
            [this, appWeak](int index)
            {
                if (index >= 0 && index < _documents.size())
                {
                    auto document = _documents[index];
                    const auto i = _documentTabs.find(document);
                    if (i != _documentTabs.end())
                    {
                        _tabWidget->removeTab(i->second);
                        _documentTabs.erase(i);
                    }
                }
            });

        _documentObserver = dtk::ValueObserver<int>::create(
            app->getDocumentsModel()->observeCurrentIndex(),
            [this](int index)
            {
                _tabWidget->setCurrentTab(index);
            });
    }

    Window::~Window()
    {}

    std::shared_ptr<Window> Window::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::string& name,
        const dtk::Size2I& size)
    {
        auto out = std::shared_ptr<Window>(new Window);
        out->_init(context, app, name, size);
        return out;
    }

    void Window::setGeometry(const dtk::Box2I& value)
    {
        dtk::Window::setGeometry(value);
        _layout->setGeometry(value);
    }

    void Window::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        dtk::Window::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void Window::keyPressEvent(dtk::KeyEvent& event)
    {
        event.accept = _menuBar->shortcut(event.key, event.modifiers);
    }

    void Window::keyReleaseEvent(dtk::KeyEvent& event)
    {
        event.accept = true;
    }

    void Window::_drop(const std::vector<std::string>& value)
    {
        if (auto context = _getContext().lock())
        {
            if (auto app = _app.lock())
            {
                std::vector<std::string> errors;
                for (const auto& i : value)
                {
                    try
                    {
                        app->getDocumentsModel()->open(i);
                    }
                    catch (const std::exception& e)
                    {
                        errors.push_back(e.what());
                    }
                }
                if (!errors.empty())
                {
                    context->getSystem<dtk::MessageDialogSystem>()->message(
                        "ERROR",
                        dtk::join(errors, '\n'),
                        getWindow());
                }
            }
        }
    }
}