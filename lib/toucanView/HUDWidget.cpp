// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "HUDWidget.h"

#include "App.h"
#include "FilesModel.h"

#include <feather-tk/ui/Spacer.h>
#include <feather-tk/core/Format.h>

namespace toucan
{
    void HUDWidget::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::IWidget::_init(context, "toucan::HUDWidget", parent);

        _file = file;

        _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
        _layout->setMarginRole(feather_tk::SizeRole::MarginSmall);
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        auto hLayout = feather_tk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        _labels["Path"] = feather_tk::Label::create(
            context,
            file->getPath().filename().string(),
            hLayout);
        _labels["Path"]->setFontRole(feather_tk::FontRole::Mono);
        _labels["Path"]->setMarginRole(feather_tk::SizeRole::MarginInside);
        _labels["Path"]->setBackgroundRole(feather_tk::ColorRole::Overlay);

        auto spacer = feather_tk::Spacer::create(context, feather_tk::Orientation::Horizontal, hLayout);
        spacer->setHStretch(feather_tk::Stretch::Expanding);

        const IMATH_NAMESPACE::V2i& imageSize = file->getImageSize();
        _labels["Image"] = feather_tk::Label::create(
            context,
            feather_tk::Format("{0}x{1}x{2} {3}").
            arg(imageSize.x).
            arg(imageSize.y).
            arg(file->getImageChannels()).
            arg(file->getImageDataType()),
            hLayout);
        _labels["Image"]->setFontRole(feather_tk::FontRole::Mono);
        _labels["Image"]->setMarginRole(feather_tk::SizeRole::MarginInside);
        _labels["Image"]->setBackgroundRole(feather_tk::ColorRole::Overlay);

        spacer = feather_tk::Spacer::create(context, feather_tk::Orientation::Vertical, _layout);
        spacer->setVStretch(feather_tk::Stretch::Expanding);

        hLayout = feather_tk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        spacer = feather_tk::Spacer::create(context, feather_tk::Orientation::Horizontal, hLayout);
        spacer->setHStretch(feather_tk::Stretch::Expanding);

        _labels["Time"] = feather_tk::Label::create(context, hLayout);
        _labels["Time"]->setFontRole(feather_tk::FontRole::Mono);
        _labels["Time"]->setMarginRole(feather_tk::SizeRole::MarginInside);
        _labels["Time"]->setBackgroundRole(feather_tk::ColorRole::Overlay);

        _currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
            file->getPlaybackModel()->observeCurrentTime(),
            [this](const OTIO_NS::RationalTime& value)
            {
                _currentTime = value;
                _widgetUpdate();
            });

        _timeRangeObserver = feather_tk::ValueObserver<OTIO_NS::TimeRange>::create(
            file->getPlaybackModel()->observeTimeRange(),
            [this](const OTIO_NS::TimeRange& value)
            {
                _timeRange = value;
                _widgetUpdate();
            });
    }

    HUDWidget::~HUDWidget()
    {}

    std::shared_ptr<HUDWidget> HUDWidget::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<HUDWidget>(new HUDWidget);
        out->_init(context, app, file, parent);
        return out;
    }

    void HUDWidget::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void HUDWidget::_widgetUpdate()
    {
        _labels["Time"]->setText(
            feather_tk::Format("{0} / {1} - {2} @ {4}").
            arg(_currentTime.value()).
            arg(_timeRange.start_time().value()).
            arg(_timeRange.end_time_inclusive().value()).
            arg(_timeRange.duration().rate()));
    }
}
