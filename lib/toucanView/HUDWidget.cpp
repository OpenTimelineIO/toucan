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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::IWidget::_init(context, "toucan::HUDWidget", parent);

        _file = file;

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setMarginRole(ftk::SizeRole::MarginSmall);
        _layout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        auto hLayout = ftk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        _labels["Path"] = ftk::Label::create(
            context,
            file->getPath().filename().string(),
            hLayout);
        _labels["Path"]->setFontRole(ftk::FontRole::Mono);
        _labels["Path"]->setMarginRole(ftk::SizeRole::MarginInside);
        _labels["Path"]->setBackgroundRole(ftk::ColorRole::Overlay);

        auto spacer = ftk::Spacer::create(context, ftk::Orientation::Horizontal, hLayout);
        spacer->setHStretch(ftk::Stretch::Expanding);

        const IMATH_NAMESPACE::V2i& imageSize = file->getImageSize();
        _labels["Image"] = ftk::Label::create(
            context,
            ftk::Format("{0}x{1}x{2} {3}").
            arg(imageSize.x).
            arg(imageSize.y).
            arg(file->getImageChannels()).
            arg(file->getImageDataType()),
            hLayout);
        _labels["Image"]->setFontRole(ftk::FontRole::Mono);
        _labels["Image"]->setMarginRole(ftk::SizeRole::MarginInside);
        _labels["Image"]->setBackgroundRole(ftk::ColorRole::Overlay);

        spacer = ftk::Spacer::create(context, ftk::Orientation::Vertical, _layout);
        spacer->setVStretch(ftk::Stretch::Expanding);

        hLayout = ftk::HorizontalLayout::create(context, _layout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        spacer = ftk::Spacer::create(context, ftk::Orientation::Horizontal, hLayout);
        spacer->setHStretch(ftk::Stretch::Expanding);

        _labels["Time"] = ftk::Label::create(context, hLayout);
        _labels["Time"]->setFontRole(ftk::FontRole::Mono);
        _labels["Time"]->setMarginRole(ftk::SizeRole::MarginInside);
        _labels["Time"]->setBackgroundRole(ftk::ColorRole::Overlay);

        _currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
            file->getPlaybackModel()->observeCurrentTime(),
            [this](const OTIO_NS::RationalTime& value)
            {
                _currentTime = value;
                _widgetUpdate();
            });

        _timeRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
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
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<HUDWidget>(new HUDWidget);
        out->_init(context, app, file, parent);
        return out;
    }

    void HUDWidget::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void HUDWidget::_widgetUpdate()
    {
        _labels["Time"]->setText(
            ftk::Format("{0} / {1} - {2} @ {4}").
            arg(_currentTime.value()).
            arg(_timeRange.start_time().value()).
            arg(_timeRange.end_time_inclusive().value()).
            arg(_timeRange.duration().rate()));
    }
}
