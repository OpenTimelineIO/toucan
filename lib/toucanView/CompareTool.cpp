// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "CompareTool.h"

#include "App.h"
#include "SelectionModel.h"

#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/Spacer.h>
#include <feather-tk/core/String.h>

namespace toucan
{
    void CompareWidget::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<IWidget>& parent)
    {
        IWidget::_init(context, "toucan::CompareWidget", parent);

        _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);

        _button = feather_tk::ToolButton::create(context, _layout);
        std::string text = file->getPath().filename().string();
        _button->setText(feather_tk::elide(text));
        _button->setHStretch(feather_tk::Stretch::Expanding);
        _button->setTooltip(text);

        _bButton = feather_tk::ToolButton::create(context, _layout);
        _bButton->setText("B");
        _bButton->setCheckable(true);
        _bButton->setTooltip("Set the B file for comparison");
    }

    CompareWidget::~CompareWidget()
    {}

    std::shared_ptr<CompareWidget> CompareWidget::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<IWidget>& parent)
    {
        std::shared_ptr<CompareWidget> out(new CompareWidget);
        out->_init(context, file, parent);
        return out;
    }

    void CompareWidget::setCurrent(bool value)
    {
        _button->setButtonRole(value ? feather_tk::ColorRole::Checked : feather_tk::ColorRole::None);
    }

    void CompareWidget::setCurrentCallback(const std::function<void(void)>& callback)
    {
        _button->setClickedCallback(
            [callback]
            {
                callback();
            });
    }

    void CompareWidget::setB(bool value)
    {
        _bButton->setChecked(value);
    }

    void CompareWidget::setBCallback(const std::function<void(bool)>& value)
    {
        _bButton->setCheckedCallback(value);
    }

    void CompareWidget::setGeometry(const feather_tk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void CompareWidget::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void CompareTool::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::CompareTool", "Compare", parent);

        _filesModel = app->getFilesModel();

        _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(feather_tk::SizeRole::None);

        _scrollWidget = feather_tk::ScrollWidget::create(context, feather_tk::ScrollType::Both, _layout);
        _scrollWidget->setBorder(false);
        _scrollWidget->setVStretch(feather_tk::Stretch::Expanding);

        _widgetLayout = feather_tk::VerticalLayout::create(context);
        _widgetLayout->setSpacingRole(feather_tk::SizeRole::SpacingTool);
        _scrollWidget->setWidget(_widgetLayout);

        feather_tk::Divider::create(context, feather_tk::Orientation::Vertical, _layout);

        _bottomLayout = feather_tk::GridLayout::create(context, _layout);
        _bottomLayout->setMarginRole(feather_tk::SizeRole::MarginSmall);
        _bottomLayout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);

        auto label = feather_tk::Label::create(context, "Mode:", _bottomLayout);
        _bottomLayout->setGridPos(label, 0, 0);
        _modeComboBox = feather_tk::ComboBox::create(context, getCompareModeLabels(), _bottomLayout);
        _modeComboBox->setHStretch(feather_tk::Stretch::Expanding);
        _modeComboBox->setTooltip("Set the comparison mode");
        _bottomLayout->setGridPos(_modeComboBox, 0, 1);

        label = feather_tk::Label::create(context, "Start time:", _bottomLayout);
        _bottomLayout->setGridPos(label, 1, 0);
        _startTimeCheckBox = feather_tk::CheckBox::create(context, _bottomLayout);
        _startTimeCheckBox->setHStretch(feather_tk::Stretch::Expanding);
        _startTimeCheckBox->setTooltip("Match the A and B start times");
        _bottomLayout->setGridPos(_startTimeCheckBox, 1, 1);

        label = feather_tk::Label::create(context, "Resize:", _bottomLayout);
        _bottomLayout->setGridPos(label, 2, 0);
        _resizeCheckBox = feather_tk::CheckBox::create(context, _bottomLayout);
        _resizeCheckBox->setHStretch(feather_tk::Stretch::Expanding);
        _resizeCheckBox->setTooltip("Resize the B image to match the A size");
        _bottomLayout->setGridPos(_resizeCheckBox, 2, 1);

        _modeComboBox->setIndexCallback(
            [this](int value)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.mode = static_cast<CompareMode>(value);
                _filesModel->setCompareOptions(options);
            });

        _startTimeCheckBox->setCheckedCallback(
            [this](bool value)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.startTime = value;
                _filesModel->setCompareOptions(options);
            });

        _resizeCheckBox->setCheckedCallback(
            [this](bool value)
            {
                CompareOptions options = _filesModel->getCompareOptions();
                options.resize = value;
                _filesModel->setCompareOptions(options);
            });

        _filesObserver = feather_tk::ListObserver<std::shared_ptr<File> >::create(
            _filesModel->observeFiles(),
            [this](const std::vector<std::shared_ptr<File> >& value)
            {
                _files = value;
                _widgetUpdate();
                _indexUpdate();
            });

        _fileIndexObserver = feather_tk::ValueObserver<int>::create(
            _filesModel->observeCurrentIndex(),
            [this](int value)
            {
                _currentIndex = value;
                _indexUpdate();
            });

        _bIndexObserver = feather_tk::ValueObserver<int>::create(
            _filesModel->observeBIndex(),
            [this](int value)
            {
                _bIndex = value;
                _indexUpdate();
            });
        
        _compareOptionsObserver = feather_tk::ValueObserver<CompareOptions>::create(
            _filesModel->observeCompareOptions(),
            [this](const CompareOptions& value)
            {
                _modeComboBox->setCurrentIndex(static_cast<int>(value.mode));
                _startTimeCheckBox->setChecked(value.startTime);
                _resizeCheckBox->setChecked(value.resize);
            });
    }

    CompareTool::~CompareTool()
    {}

    std::shared_ptr<CompareTool> CompareTool::create(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<CompareTool>(new CompareTool);
        out->_init(context, app, parent);
        return out;
    }

    void CompareTool::setGeometry(const feather_tk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void CompareTool::sizeHintEvent(const feather_tk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void CompareTool::_widgetUpdate()
    {
        for (const auto& widget : _widgets)
        {
            widget->setParent(nullptr);
        }
        _widgets.clear();

        auto context = getContext();
        for (size_t i = 0; i < _files.size(); ++i)
        {
            auto widget = CompareWidget::create(context, _files[i], _widgetLayout);
            widget->setCurrent(i == _currentIndex);
            widget->setB(i == _bIndex);
            widget->setCurrentCallback(
                [this, i]
                {
                    _filesModel->setCurrentIndex(i);
                });
            widget->setBCallback(
                [this, i](bool value)
                {
                    _filesModel->setBIndex(value ? i : -1);
                });
            _widgets.push_back(widget);
        }
    }

    void CompareTool::_indexUpdate()
    {
        for (size_t i = 0; i < _widgets.size(); ++i)
        {
            _widgets[i]->setCurrent(i == _currentIndex);
            _widgets[i]->setB(i == _bIndex);
        }
    }
}
