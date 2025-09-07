// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ExportTool.h"

#include "App.h"
#include "FilesModel.h"
#include "PlaybackModel.h"

#include <toucanRender/Util.h>

#include <feather-tk/ui/DialogSystem.h>
#include <feather-tk/ui/GridLayout.h>
#include <feather-tk/ui/Settings.h>
#include <feather-tk/ui/Window.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>

#include <OpenImageIO/imagebufalgo.h>

#include <nlohmann/json.hpp>

#include <sstream>

namespace toucan
{
    void ExportWidget::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IWidget::_init(context, "toucan::ExportWidget", parent);

        _settings = app->getSettings();
        _host = app->getHost();
        _movieCodecs = ffmpeg::getVideoCodecStrings();

        SettingsValues settingsValues;
        _initSettings(context, settingsValues);
        _initCommonUI(context, settingsValues);
        _initImageUI(context, settingsValues);
        _initMovieUI(context, settingsValues);
        
        _widgetUpdate();

        _timer = ftk::Timer::create(context);
        _timer->setRepeating(true);

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _exportSequenceButton->setEnabled(_file.get());
                _exportFrameButton->setEnabled(_file.get());
                _exportMovieButton->setEnabled(_file.get());
            });
    }

    ExportWidget::~ExportWidget()
    {
        nlohmann::json json;
        json["Dir"] = _dirEdit->getPath().string();
        json["SizeChoice"] = _sizeComboBox->getCurrentIndex();
        json["CustomSize"] = { _widthEdit->getValue(), _heightEdit->getValue() };
        json["CurrentTab"] = _tabWidget->getCurrentTab();
        json["ImageBaseName"] = _imageBaseNameEdit->getText();
        json["ImagePadding"] = _imagePaddingEdit->getValue();
        json["ImageExtension"] = _imageExtensionEdit->getText();
        json["MovieBaseName"] = _movieBaseNameEdit->getText();
        json["MovieExtension"] = _movieExtensionEdit->getText();
        json["MovieCodec"] = ffmpeg::toString(
            static_cast<ffmpeg::VideoCodec>(_movieCodecComboBox->getCurrentIndex()));
        auto context = getContext();
        _settings->set("/ExportWidget", json);
    }

    std::shared_ptr<ExportWidget> ExportWidget::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ExportWidget>(new ExportWidget);
        out->_init(context, app, parent);
        return out;
    }

    void ExportWidget::setGeometry(const ftk::Box2I& value)
    {
        IWidget::setGeometry(value);
        _layout->setGeometry(value);
    }

    void ExportWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IWidget::sizeHintEvent(event);
        _setSizeHint(_layout->getSizeHint());
    }

    void ExportWidget::_initSettings(
        const std::shared_ptr<ftk::Context>& context,
        SettingsValues& settingsValues)
    {
        try
        {
            nlohmann::json json;
            _settings->get("/ExportWidget", json);
            auto i = json.find("Dir");
            if (i != json.end() && i->is_string())
            {
                settingsValues.dir = i->get<std::string>();
            }
            i = json.find("SizeChoice");
            if (i != json.end() && i->is_number())
            {
                settingsValues.sizeChoice = i->get<int>();
            }
            i = json.find("CustomSize");
            if (i != json.end() &&
                i->is_array() &&
                i->size() == 2 &&
                (*i)[0].is_number() &&
                (*i)[1].is_number())
            {
                settingsValues.customSize.w = (*i)[0].get<int>();
                settingsValues.customSize.h = (*i)[1].get<int>();
            }
            i = json.find("CurrentTab");
            if (i != json.end() && i->is_number())
            {
                settingsValues.currentTab = i->get<int>();
            }
            i = json.find("ImageBaseName");
            if (i != json.end() && i->is_string())
            {
                settingsValues.imageBaseName = i->get<std::string>();
            }
            i = json.find("ImagePadding");
            if (i != json.end() && i->is_number())
            {
                settingsValues.imagePadding = i->get<int>();
            }
            i = json.find("ImageExtension");
            if (i != json.end() && i->is_string())
            {
                settingsValues.imageExtension = i->get<std::string>();
            }
            i = json.find("MovieBaseName");
            if (i != json.end() && i->is_string())
            {
                settingsValues.movieBaseName = i->get<std::string>();
            }
            i = json.find("MovieExtension");
            if (i != json.end() && i->is_string())
            {
                settingsValues.movieExtension = i->get<std::string>();
            }
            i = json.find("MovieCodec");
            if (i != json.end() && i->is_string())
            {
                settingsValues.movieCodec = i->get<std::string>();
            }
        }
        catch (const std::exception&)
        {}
    }

    void ExportWidget::_initCommonUI(
        const std::shared_ptr<ftk::Context>& context,
        const SettingsValues& settingsValues)
    {
        _layout = ftk::VerticalLayout::create(context, shared_from_this());
        _layout->setSpacingRole(ftk::SizeRole::None);

        auto vLayout = ftk::VerticalLayout::create(context, _layout);
        vLayout->setMarginRole(ftk::SizeRole::Margin);

        auto gridLayout = ftk::GridLayout::create(context, vLayout);

        auto label = ftk::Label::create(context, "Directory:", gridLayout);
        gridLayout->setGridPos(label, 0, 0);
        _dirEdit = ftk::FileEdit::create(context, gridLayout);
        _dirEdit->setPath(settingsValues.dir);
        _dirEdit->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_dirEdit, 0, 1);

        label = ftk::Label::create(context, "Image size:", gridLayout);
        gridLayout->setGridPos(label, 1, 0);
        auto hLayout = ftk::HorizontalLayout::create(context, gridLayout);
        hLayout->setSpacingRole(ftk::SizeRole::SpacingSmall);
        gridLayout->setGridPos(hLayout, 1, 1);
        _sizeComboBox = ftk::ComboBox::create(
            context,
            std::vector<std::string>{ "Default", "Custom" },
            hLayout);
        _sizeComboBox->setCurrentIndex(settingsValues.sizeChoice);
        _sizeComboBox->setHStretch(ftk::Stretch::Expanding);
        _widthEdit = ftk::IntEdit::create(context, hLayout);
        _widthEdit->setRange(ftk::RangeI(1, 15360));
        _widthEdit->setValue(settingsValues.customSize.w);
        _heightEdit = ftk::IntEdit::create(context, hLayout);
        _heightEdit->setRange(ftk::RangeI(1, 15360));
        _heightEdit->setValue(settingsValues.customSize.h);

        ftk::Divider::create(context, ftk::Orientation::Vertical, _layout);

        _tabWidget = ftk::TabWidget::create(context, _layout);

        _sizeComboBox->setIndexCallback(
            [this](int value)
            {
                _widgetUpdate();
            });
    }

    void ExportWidget::_initImageUI(
        const std::shared_ptr<ftk::Context>& context,
        const SettingsValues& settingsValues)
    {
        auto vLayout = ftk::VerticalLayout::create(context);
        vLayout->setMarginRole(ftk::SizeRole::Margin);
        _tabWidget->addTab("Images", vLayout);

        auto gridLayout = ftk::GridLayout::create(context, vLayout);

        auto label = ftk::Label::create(context, "Base name:", gridLayout);
        gridLayout->setGridPos(label, 0, 0);
        _imageBaseNameEdit = ftk::LineEdit::create(context, gridLayout);
        _imageBaseNameEdit->setText(settingsValues.imageBaseName);
        _imageBaseNameEdit->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_imageBaseNameEdit, 0, 1);

        label = ftk::Label::create(context, "Number padding:", gridLayout);
        gridLayout->setGridPos(label, 1, 0);
        _imagePaddingEdit = ftk::IntEdit::create(context, gridLayout);
        _imagePaddingEdit->setRange(ftk::RangeI(0, 9));
        _imagePaddingEdit->setValue(settingsValues.imagePadding);
        gridLayout->setGridPos(_imagePaddingEdit, 1, 1);

        label = ftk::Label::create(context, "Extension:", gridLayout);
        gridLayout->setGridPos(label, 2, 0);
        _imageExtensionEdit = ftk::LineEdit::create(context, gridLayout);
        _imageExtensionEdit->setText(settingsValues.imageExtension);
        _imageExtensionEdit->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_imageExtensionEdit, 2, 1);

        label = ftk::Label::create(context, "Filename:", gridLayout);
        gridLayout->setGridPos(label, 3, 0);
        _imageFilenameLabel = ftk::Label::create(context, gridLayout);
        _imageFilenameLabel->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_imageFilenameLabel, 3, 1);

        ftk::Divider::create(context, ftk::Orientation::Vertical, vLayout);

        _exportSequenceButton = ftk::PushButton::create(
            context,
            "Export Sequence",
            vLayout);

        _exportFrameButton = ftk::PushButton::create(
            context,
            "Export Frame",
            vLayout);

        _imageBaseNameEdit->setTextChangedCallback(
            [this](const std::string&)
            {
                _widgetUpdate();
            });

        _imagePaddingEdit->setCallback(
            [this](int)
            {
                _widgetUpdate();
            });

        _imageExtensionEdit->setTextChangedCallback(
            [this](const std::string&)
            {
                _widgetUpdate();
            });

        _exportSequenceButton->setClickedCallback(
            [this]
            {
                _export(ExportType::Sequence);
            });

        _exportFrameButton->setClickedCallback(
            [this]
            {
                _export(ExportType::Frame);
            });
    }

    void ExportWidget::_initMovieUI(
        const std::shared_ptr<ftk::Context>& context,
        const SettingsValues& settingsValues)
    {
        auto vLayout = ftk::VerticalLayout::create(context);
        vLayout->setMarginRole(ftk::SizeRole::Margin);
        _tabWidget->addTab("Movie", vLayout);
        _tabWidget->setCurrentTab(settingsValues.currentTab);

        auto gridLayout = ftk::GridLayout::create(context, vLayout);

        auto label = ftk::Label::create(context, "Base name:", gridLayout);
        gridLayout->setGridPos(label, 0, 0);
        _movieBaseNameEdit = ftk::LineEdit::create(context, gridLayout);
        _movieBaseNameEdit->setText(settingsValues.movieBaseName);
        _movieBaseNameEdit->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_movieBaseNameEdit, 0, 1);

        label = ftk::Label::create(context, "Extension:", gridLayout);
        gridLayout->setGridPos(label, 1, 0);
        _movieExtensionEdit = ftk::LineEdit::create(context, gridLayout);
        _movieExtensionEdit->setText(settingsValues.movieExtension);
        _movieExtensionEdit->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_movieExtensionEdit, 1, 1);

        label = ftk::Label::create(context, "Filename:", gridLayout);
        gridLayout->setGridPos(label, 2, 0);
        _movieFilenameLabel = ftk::Label::create(context, gridLayout);
        _movieFilenameLabel->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_movieFilenameLabel, 2, 1);

        label = ftk::Label::create(context, "Codec:", gridLayout);
        gridLayout->setGridPos(label, 3, 0);
        _movieCodecComboBox = ftk::ComboBox::create(context, _movieCodecs, gridLayout);
        ffmpeg::VideoCodec ffmpegVideoCodec = ffmpeg::VideoCodec::First;
        ffmpeg::fromString(settingsValues.movieCodec, ffmpegVideoCodec);
        _movieCodecComboBox->setCurrentIndex(static_cast<int>(ffmpegVideoCodec));
        _movieCodecComboBox->setHStretch(ftk::Stretch::Expanding);
        gridLayout->setGridPos(_movieCodecComboBox, 3, 1);

        ftk::Divider::create(context, ftk::Orientation::Vertical, vLayout);

        _exportMovieButton = ftk::PushButton::create(
            context,
            "Export Movie",
            vLayout);

        _movieBaseNameEdit->setTextChangedCallback(
            [this](const std::string&)
            {
                _widgetUpdate();
            });

        _movieExtensionEdit->setTextChangedCallback(
            [this](const std::string&)
            {
                _widgetUpdate();
            });

        _exportMovieButton->setClickedCallback(
            [this]
            {
                _export(ExportType::Movie);
            });
    }

    void ExportWidget::_export(ExportType type)
    {
        if (!_file)
            return;
        try
        {
            _graph = std::make_shared<ImageGraph>(
                getContext(),
                _file->getPath(),
                _file->getTimelineWrapper());
            _imageSize = _graph->getImageSize();
            _outputSize = _imageSize;
            if (_sizeComboBox->getCurrentIndex() != 0)
            {
                _outputSize.x = _widthEdit->getValue();
                _outputSize.y = _heightEdit->getValue();
            }

            switch (type)
            {
            case ExportType::Sequence:
                _timeRange = _file->getPlaybackModel()->getInOutRange();
                break;
            case ExportType::Frame:
                _timeRange = OTIO_NS::TimeRange(
                    _file->getPlaybackModel()->getCurrentTime(),
                    OTIO_NS::RationalTime(1.0, _file->getPlaybackModel()->getTimeRange().duration().rate()));
                break;
            case ExportType::Movie:
            {
                _timeRange = _file->getPlaybackModel()->getInOutRange();
                const std::string baseName = _movieBaseNameEdit->getText();
                const std::string extension = _movieExtensionEdit->getText();
                const std::filesystem::path path = _dirEdit->getPath() / (baseName + extension);
                ffmpeg::VideoCodec videoCodec = ffmpeg::VideoCodec::First;
                ffmpeg::fromString(_movieCodecs[_movieCodecComboBox->getCurrentIndex()], videoCodec);
                _ffWrite = std::make_shared<ffmpeg::Write>(
                    path,
                    OIIO::ImageSpec(_outputSize.x, _outputSize.y, 3),
                    _timeRange,
                    videoCodec);
                break;
            }
            default: break;
            }

            _dialog = ftk::ProgressDialog::create(
                getContext(),
                "Export",
                "Exporting:");
            _dialog->setMessage(ftk::Format("{0} / {1}").arg(0).arg(_timeRange.duration().value()));
            _dialog->setCloseCallback(
                [this]
                {
                    _timer->stop();
                    _graph.reset();
                    _ffWrite.reset();
                    _dialog.reset();
                });
            _dialog->open(getWindow());

            _time = _timeRange.start_time();
            _timer->start(
                std::chrono::microseconds(0),
                [this]
                {
                    _exportFrame();
                });
        }
        catch (const std::exception& e)
        {
            auto dialogSystem = getContext()->getSystem<ftk::DialogSystem>();
            dialogSystem->message("ERROR", e.what(), getWindow());
        }
    }

    void ExportWidget::_exportFrame()
    {
        if (auto node = _graph->exec(_host, _time))
        {
            auto buf = node->exec(_time);
            if (_outputSize != _imageSize)
            {
                buf = OIIO::ImageBufAlgo::resize(
                    buf,
                    "",
                    0.F,
                    OIIO::ROI(0, _outputSize.x, 0, _outputSize.y));
            }
            try
            {
                if (_ffWrite)
                {
                    _ffWrite->writeImage(buf, _time);
                }
                else
                {
                    const std::string fileName = getSequenceFrame(
                        _dirEdit->getPath().string(),
                        _imageBaseNameEdit->getText(),
                        _time.to_frames(),
                        _imagePaddingEdit->getValue(),
                        _imageExtensionEdit->getText());
                    buf.write(fileName);
                }
            }
            catch (const std::exception& e)
            {
                if (_dialog)
                {
                    _dialog->close();
                }
                getContext()->getSystem<ftk::DialogSystem>()->message(
                    "ERROR",
                    e.what(),
                    getWindow());
            }
        }

        if (_dialog)
        {
            const OTIO_NS::RationalTime end = _timeRange.end_time_inclusive();
            if (_time < end)
            {
                _time += OTIO_NS::RationalTime(1.0, _timeRange.duration().rate());
                const OTIO_NS::RationalTime duration = _timeRange.duration();
                const double v = duration.value() > 0.0 ?
                    (_time - _timeRange.start_time()).value() / static_cast<double>(duration.value()) :
                    0.0;
                _dialog->setValue(v);
                _dialog->setMessage(ftk::Format("{0} / {1}").
                    arg((_time - _timeRange.start_time()).value()).
                    arg(_timeRange.duration().value()));
            }
            else
            {
                _dialog->close();
            }
        }
    }

    void ExportWidget::_widgetUpdate()
    {
        const int index = _sizeComboBox->getCurrentIndex();
        _widthEdit->setVisible(index != 0);
        _heightEdit->setVisible(index != 0);

        _imageFilenameLabel->setText(getSequenceFrame(
            _dirEdit->getPath().string(),
            _imageBaseNameEdit->getText(),
            0,
            _imagePaddingEdit->getValue(),
            _imageExtensionEdit->getText()));

        _movieFilenameLabel->setText(
            _movieBaseNameEdit->getText() +
            _movieExtensionEdit->getText());
    }

    void ExportTool::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        IToolWidget::_init(context, app, "toucan::ExportTool", "Export", parent);

        _scrollWidget = ftk::ScrollWidget::create(context, ftk::ScrollType::Both, shared_from_this());
        _scrollWidget->setBorder(false);

        _widget = ExportWidget::create(context, app);
        _scrollWidget->setWidget(_widget);
    }

    ExportTool::~ExportTool()
    {}

    std::shared_ptr<ExportTool> ExportTool::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<ExportTool>(new ExportTool);
        out->_init(context, app, parent);
        return out;
    }

    void ExportTool::setGeometry(const ftk::Box2I& value)
    {
        IToolWidget::setGeometry(value);
        _scrollWidget->setGeometry(value);
    }

    void ExportTool::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        IToolWidget::sizeHintEvent(event);
        _setSizeHint(_scrollWidget->getSizeHint());
    }
}
