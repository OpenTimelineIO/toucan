// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "BackgroundTool.h"

#include "App.h"

#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/Spacer.h>
#include <feather-tk/core/String.h>

namespace toucan
{
    void BackgroundTool::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::BackgroundTool", "Background", parent);

        _model = app->getGlobalViewModel();

        _scrollWidget = feather_tk::ScrollWidget::create(context, feather_tk::ScrollType::Both, shared_from_this());
        _scrollWidget->setBorder(false);

        _layout = feather_tk::VerticalLayout::create(context);
        _layout->setMarginRole(feather_tk::SizeRole::MarginSmall);
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);
        _scrollWidget->setWidget(_layout);

        _comboBox = feather_tk::ComboBox::create(context, getViewBackgroundLabels(), _layout);
        
        feather_tk::Divider::create(context, feather_tk::Orientation::Vertical, _layout);

        _solidColorWidget = feather_tk::ColorWidget::create(context, _layout);

        _checkersColor0Widget = feather_tk::ColorWidget::create(context, _layout);

        _checkersColor1Widget = feather_tk::ColorWidget::create(context, _layout);

        _checkersSizeSlider = feather_tk::IntEditSlider::create(context, _layout);
        _checkersSizeSlider->setRange(feather_tk::RangeI(10, 100));

        _comboBox->setIndexCallback(
            [this](int value)
            {
                if (_model)
                {
                    auto options = _model->getOptions();
                    options.background = static_cast<ViewBackground>(value);
                    _model->setOptions(options);
                }
            });

        _solidColorWidget->setCallback(
            [this](const feather_tk::Color4F& value)
            {
                if (_model)
                {
                    auto options = _model->getOptions();
                    options.solidColor = value;
                    _model->setOptions(options);
                }
            });

        _checkersColor0Widget->setCallback(
            [this](const feather_tk::Color4F& value)
            {
                if (_model)
                {
                    auto options = _model->getOptions();
                    options.checkersColor0 = value;
                    _model->setOptions(options);
                }
            });

        _checkersColor1Widget->setCallback(
            [this](const feather_tk::Color4F& value)
            {
                if (_model)
                {
                    auto options = _model->getOptions();
                    options.checkersColor1 = value;
                    _model->setOptions(options);
                }
            });

        _checkersSizeSlider->setCallback(
            [this](int value)
            {
                if (_model)
                {
                    auto options = _model->getOptions();
                    options.checkersSize = value;
                    _model->setOptions(options);
                }
            });

        _optionsObserver = feather_tk::ValueObserver<GlobalViewOptions>::create(
            _model->observeOptions(),
            [this](const GlobalViewOptions& value)
            {
                _options = value;
                _widgetUpdate();
            });
    }

    BackgroundTool::~BackgroundTool()
    {}

    std::shared_ptr<BackgroundTool> BackgroundTool::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<BackgroundTool>(new BackgroundTool);
        out->_init(context, app, parent);
        return out;
    }

    void BackgroundTool::setGeometry(const feather_tk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _scrollWidget->setGeometry(value);
    }

    void BackgroundTool::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_scrollWidget->getSizeHint());
    }

    void BackgroundTool::_widgetUpdate()
    {
        _comboBox->setCurrentIndex(static_cast<int>(_options.background));

        _solidColorWidget->setVisible(ViewBackground::Solid == _options.background);
        _solidColorWidget->setColor(_options.solidColor);

        _checkersColor0Widget->setVisible(ViewBackground::Checkers == _options.background);
        _checkersColor0Widget->setColor(_options.checkersColor0);

        _checkersColor1Widget->setVisible(ViewBackground::Checkers == _options.background);
        _checkersColor1Widget->setColor(_options.checkersColor1);

        _checkersSizeSlider->setVisible(ViewBackground::Checkers == _options.background);
        _checkersSizeSlider->setValue(_options.checkersSize);
    }
}
