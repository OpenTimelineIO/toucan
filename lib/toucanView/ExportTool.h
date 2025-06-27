// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IToolWidget.h>

#include <toucanRender/FFmpegWrite.h>
#include <toucanRender/ImageGraph.h>

#include <feather-tk/ui/ComboBox.h>
#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/FileEdit.h>
#include <feather-tk/ui/IntEdit.h>
#include <feather-tk/ui/Label.h>
#include <feather-tk/ui/LineEdit.h>
#include <feather-tk/ui/ProgressDialog.h>
#include <feather-tk/ui/PushButton.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/ui/Settings.h>
#include <feather-tk/ui/TabWidget.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/Timer.h>

namespace toucan
{
    class File;

    //! Export widget.
    class ExportWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ExportWidget();

        //! Create a new widget.
        static std::shared_ptr<ExportWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        struct SettingsValues
        {
            std::string dir;
            int sizeChoice = 0;
            feather_tk::Size2I customSize = feather_tk::Size2I(1920, 1080);
            int currentTab = 0;
            std::string imageBaseName = "render.";
            int imagePadding = 0;
            std::string imageExtension = ".tif";
            std::string movieBaseName = "render";
            std::string movieExtension = ".mov";
            std::string movieCodec = "MJPEG";
        };

        void _initSettings(const std::shared_ptr<feather_tk::Context>&, SettingsValues&);
        void _initCommonUI(const std::shared_ptr<feather_tk::Context>&, const SettingsValues&);
        void _initImageUI(const std::shared_ptr<feather_tk::Context>&, const SettingsValues&);
        void _initMovieUI(const std::shared_ptr<feather_tk::Context>&, const SettingsValues&);

        enum class ExportType
        {
            Sequence,
            Frame,
            Movie
        };

        void _export(ExportType);
        void _exportFrame();

        void _widgetUpdate();

        std::shared_ptr<feather_tk::Settings> _settings;
        std::shared_ptr<ImageEffectHost> _host;
        std::shared_ptr<File> _file;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _time;
        std::shared_ptr<ImageGraph> _graph;
        IMATH_NAMESPACE::V2d _imageSize = IMATH_NAMESPACE::V2d(0, 0);
        IMATH_NAMESPACE::V2d _outputSize = IMATH_NAMESPACE::V2d(0, 0);
        std::vector<std::string> _movieCodecs;
        std::shared_ptr<ffmpeg::Write> _ffWrite;

        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::shared_ptr<feather_tk::FileEdit> _dirEdit;
        std::shared_ptr<feather_tk::ComboBox> _sizeComboBox;
        std::shared_ptr<feather_tk::IntEdit> _widthEdit;
        std::shared_ptr<feather_tk::IntEdit> _heightEdit;
        std::shared_ptr<feather_tk::TabWidget> _tabWidget;
        std::shared_ptr<feather_tk::LineEdit> _imageBaseNameEdit;
        std::shared_ptr<feather_tk::IntEdit> _imagePaddingEdit;
        std::shared_ptr<feather_tk::LineEdit> _imageExtensionEdit;
        std::shared_ptr<feather_tk::Label> _imageFilenameLabel;
        std::shared_ptr<feather_tk::PushButton> _exportSequenceButton;
        std::shared_ptr<feather_tk::PushButton> _exportFrameButton;
        std::shared_ptr<feather_tk::LineEdit> _movieBaseNameEdit;
        std::shared_ptr<feather_tk::LineEdit> _movieExtensionEdit;
        std::shared_ptr<feather_tk::ComboBox> _movieCodecComboBox;
        std::shared_ptr<feather_tk::Label> _movieFilenameLabel;
        std::shared_ptr<feather_tk::PushButton> _exportMovieButton;
        std::shared_ptr<feather_tk::ProgressDialog> _dialog;

        std::shared_ptr<feather_tk::Timer> _timer;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
    };

    //! Export tool.
    class ExportTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ExportTool();

        //! Create a new tool.
        static std::shared_ptr<ExportTool> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<ExportWidget> _widget;
    };
}

