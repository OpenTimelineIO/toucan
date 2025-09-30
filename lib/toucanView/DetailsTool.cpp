// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "DetailsTool.h"

#include "App.h"
#include "FilesModel.h"
#include "PlaybackModel.h"

#include <ftk/Ui/Divider.h>
#include <ftk/Ui/Spacer.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <opentimelineio/marker.h>

namespace toucan
{
    void DetailsWidget::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const SelectionItem& item,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IWidget::_init(context, "toucan::DetailsWidget", parent);

        _file = file;
        _item = item;

        _bellows = ftk::Bellows::create(context, item.object->name(), shared_from_this());
        _bellows->setOpen(true);
        auto hLayout = ftk::HorizontalLayout::create(context);
        hLayout->setSpacingRole(ftk::SizeRole::None);
        _startFrameButton = ftk::ToolButton::create(context, hLayout);
        _startFrameButton->setIcon("ArrowRight");
        _startFrameButton->setTooltip("Go to the start frame");
        if (item.timeRange.duration().value() > 1.0)
        {
            _inOutButton = ftk::ToolButton::create(context, hLayout);
            _inOutButton->setIcon("FrameInOut");
            _inOutButton->setTooltip("Set the in/out points");
        }
        _bellows->setToolWidget(hLayout);

        _layout = ftk::GridLayout::create(context);
        _layout->setRowBackgroundRole(ftk::ColorRole::Base);
        _layout->setSpacingRole(ftk::SizeRole::None);
        _bellows->setWidget(_layout);

        OTIO_NS::TimeRange timeRange = item.timeRange;
        _startFrameButton->setClickedCallback(
            [this, timeRange]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setCurrentTime(timeRange.start_time());
                }
            });

        if (_inOutButton)
        {
            _inOutButton->setClickedCallback(
                [this, timeRange]
                {
                    if (_file)
                    {
                        _file->getPlaybackModel()->setInOutRange(timeRange);
                        _file->getPlaybackModel()->setCurrentTime(timeRange.start_time());
                    }
                });
        }

        _timeUnitsObserver = ftk::ValueObserver<TimeUnits>::create(
            app->getTimeUnitsModel()->observeTimeUnits(),
            [this](TimeUnits value)
            {
                _timeUnits = value;
                _textUpdate();
                _searchUpdate();
            });
    }

    DetailsWidget::~DetailsWidget()
    {}

    std::shared_ptr<DetailsWidget> DetailsWidget::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const SelectionItem& item,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<DetailsWidget>(new DetailsWidget);
        out->_init(context, app, file, item, parent);
        return out;
    }

    void DetailsWidget::setOpen(bool value)
    {
        _bellows->setOpen(value);
    }

    void DetailsWidget::setSearch(const std::string& value)
    {
        if (value == _search)
            return;
        _search = value;
        _searchUpdate();
    }

    void DetailsWidget::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _bellows->setGeometry(value);
    }

    void DetailsWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_bellows->isVisible(false) ? _bellows->getSizeHint() : ftk::Size2I());
    }

    void DetailsWidget::_textUpdate()
    {
        _text.clear();
        for (const auto& label : _labels)
        {
            label.first->setParent(nullptr);
            if (label.second)
            {
                label.second->setParent(nullptr);
            }
        }
        _labels.clear();

        _text.push_back(std::make_pair("Schema:", _item.object->schema_name()));
        _text.push_back(std::make_pair("Name:", _item.object->name()));

        if (auto item = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(_item.object))
        {
            _text.push_back(std::make_pair(
                "Enabled:",
                ftk::Format("{0}").arg(item->enabled())));
            std::string text;
            if (item->source_range().has_value())
            {
                text = toString(item->source_range().value(), _timeUnits);
            }
            _text.push_back(std::make_pair("Source range:", text));
            _text.push_back(std::make_pair("Available range:",
                toString(item->available_range(), _timeUnits)));
            _text.push_back(std::make_pair("Trimmed range:",
                toString(item->trimmed_range(), _timeUnits)));

            //! \bug Calling trimmed_range_in_parent() on a stack causes a crash?
            text.clear();
            auto stack = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Stack>(item);
            if (!stack && item->trimmed_range_in_parent().has_value())
            {
                text = toString(item->trimmed_range_in_parent().value(), _timeUnits);
            }
            _text.push_back(std::make_pair("Trimmed range in parent:", text));
        }
        else if (auto marker = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Marker>(_item.object))
        {
            _text.push_back(std::make_pair("Color:", marker->color()));
            _text.push_back(std::make_pair("Range:",
                toString(marker->marked_range(), _timeUnits)));
            _text.push_back(std::make_pair("Comment:", marker->comment()));
        }

        int row = 0;
        for (const auto& text : _text)
        {
            auto context = getContext();
            auto label = ftk::Label::create(context, text.first, _layout);
            label->setMarginRole(ftk::SizeRole::MarginSmall);
            _layout->setGridPos(label, row, 0);
            std::shared_ptr<ftk::Label> label2;
            if (!text.second.empty())
            {
                label2 = ftk::Label::create(context, text.second, _layout);
                label2->setMarginRole(ftk::SizeRole::MarginSmall);
                _layout->setGridPos(label2, row, 1);
            }
            _labels.push_back(std::make_pair(label, label2));
            ++row;
        }
    }

    void DetailsWidget::_searchUpdate()
    {
        size_t visibleCount = 0;
        for (size_t i = 0; i < _text.size() && i < _labels.size(); ++i)
        {
            bool visible = true;
            if (!_search.empty())
            {
                visible &=
                    ftk::contains(_text[i].first, _search, ftk::CaseCompare::Insensitive) ||
                    ftk::contains(_text[i].second, _search, ftk::CaseCompare::Insensitive);
            }
            _labels[i].first->setVisible(visible);
            if (_labels[i].second)
            {
                _labels[i].second->setVisible(visible);
            }
            if (visible)
            {
                ++visibleCount;
            }
        }
        _bellows->setVisible(visibleCount > 0);
    }

    void DetailsTool::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::DetailsTool", "Details", parent);

        _app = app;

        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::None);

        _scrollWidget = ftk::ScrollWidget::create(context, ftk::ScrollType::Both, _layout);
        _scrollWidget->setBorder(false);
        _scrollWidget->setVStretch(ftk::Stretch::Expanding);

        _scrollLayout = ftk::VerticalLayout::create(context);
        _scrollLayout->setSpacingRole(ftk::SizeRole::None);
        _scrollWidget->setWidget(_scrollLayout);

        ftk::Divider::create(context, ftk::Orientation::Vertical, _layout);

        _bottomLayout = ftk::HorizontalLayout::create(context, _layout);
        _bottomLayout->setMarginRole(ftk::SizeRole::MarginSmall);
        _bottomLayout->setSpacingRole(ftk::SizeRole::SpacingSmall);

        _searchBox = ftk::SearchBox::create(context, _bottomLayout);
        _searchBox->setHStretch(ftk::Stretch::Expanding);
        _searchBox->setTooltip("Search the JSON text");

        auto hLayout = ftk::HorizontalLayout::create(context, _bottomLayout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
        _openButton = ftk::ToolButton::create(context, hLayout);
        _openButton->setIcon("BellowsOpen");
        _openButton->setTooltip("Open all");
        _closeButton = ftk::ToolButton::create(context, hLayout);
        _closeButton->setIcon("BellowsClosed");
        _closeButton->setTooltip("Close all");

        _searchBox->setCallback(
            [this](const std::string& text)
            {
                for (const auto& widget : _widgets)
                {
                    widget->setSearch(text);
                }
            });

        _openButton->setClickedCallback(
            [this]
            {
                for (const auto& widget : _widgets)
                {
                    widget->setOpen(true);
                }
            });

        _closeButton->setClickedCallback(
            [this]
            {
                for (const auto& widget : _widgets)
                {
                    widget->setOpen(false);
                }
            });

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                if (file)
                {
                    _selectionObserver = ftk::ListObserver<SelectionItem>::create(
                        file->getSelectionModel()->observeSelection(),
                        [this, file](const std::vector<SelectionItem>& selection)
                        {
                            for (const auto& widget : _widgets)
                            {
                                widget->setParent(nullptr);
                            }
                            _widgets.clear();
                            auto context = getContext();
                            const std::string& search = _searchBox->getText();
                            for (const auto& item : selection)
                            {
                                auto widget = DetailsWidget::create(
                                    context,
                                    _app.lock(),
                                    file,
                                    item,
                                    _scrollLayout);
                                widget->setSearch(search);
                                _widgets.push_back(widget);
                            }
                            _openButton->setEnabled(!_widgets.empty());
                            _closeButton->setEnabled(!_widgets.empty());
                        });
                }
                else
                {
                    for (const auto& widget : _widgets)
                    {
                        widget->setParent(nullptr);
                    }
                    _widgets.clear();
                    _openButton->setEnabled(false);
                    _closeButton->setEnabled(false);
                    _selectionObserver.reset();
                }
            });
    }

    DetailsTool::~DetailsTool()
    {}

    std::shared_ptr<DetailsTool> DetailsTool::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<DetailsTool>(new DetailsTool);
        out->_init(context, app, parent);
        return out;
    }

    void DetailsTool::setGeometry(const ftk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void DetailsTool::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }
}
