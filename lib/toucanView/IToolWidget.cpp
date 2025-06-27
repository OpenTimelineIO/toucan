// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "IToolWidget.h"

namespace toucan
{
    void IToolWidget::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::string& objectName,
        const std::string& text,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IWidget::_init(context, objectName, parent);
        _app = app;
        _text = text;
    }

    IToolWidget::~IToolWidget()
    {}

    const std::string& IToolWidget::getText() const
    {
        return _text;
    }
}
