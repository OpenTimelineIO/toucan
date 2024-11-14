// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "InfoBar.h"

#include "App.h"
#include "DocumentsModel.h"

#include <dtk/core/Format.h>
#include <dtk/core/String.h>

namespace toucan
{
    void InfoBar::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        dtk::IWidget::_init(context, "toucan::InfoBar", parent);

        _layout = dtk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(dtk::SizeRole::SpacingTool);

        _label = dtk::Label::create(context, _layout);
        _label->setMarginRole(dtk::SizeRole::MarginInside);

        _documentObserver = dtk::ValueObserver<std::shared_ptr<Document> >::create(
            app->getDocumentsModel()->observeCurrent(),
            [this](const std::shared_ptr<Document>& document)
            {
                std::string text;
                std::string tooltip;
                if (document)
                {
                    const IMATH_NAMESPACE::V2i& imageSize = document->getImageSize();
                    const size_t trackCount = document->getTimeline()->otio()->find_children<OTIO_NS::Track>().size();

                    text = dtk::Format("{0}: {1}x{2}, {3} tracks").
                        arg(dtk::elide(document->getPath().filename().string())).
                        arg(imageSize.x).
                        arg(imageSize.y).
                        arg(trackCount);
                    tooltip = dtk::Format(
                        "path: {0}\n"
                        "resolution: {1}x{2}\n"
                        "tracks: {3}").
                        arg(document->getPath().string()).
                        arg(imageSize.x).
                        arg(imageSize.y).
                        arg(trackCount);
                }
                _label->setText(text);
                _label->setTooltip(tooltip);
            });
    }

    InfoBar::~InfoBar()
    {}

    std::shared_ptr<InfoBar> InfoBar::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        auto out = std::shared_ptr<InfoBar>(new InfoBar);
        out->_init(context, app, parent);
        return out;
    }

    void InfoBar::setGeometry(const dtk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void InfoBar::sizeHintEvent(const dtk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
