// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/FilesModel.h>
#include <toucanView/ViewModel.h>
#include <toucanView/IToolWidget.h>

#include <feather-tk/ui/ColorWidget.h>
#include <feather-tk/ui/ComboBox.h>
#include <feather-tk/ui/IntEditSlider.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>

namespace toucan
{
    class File;

    //! Background tool.
    class BackgroundTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~BackgroundTool();

        //! Create a new tool.
        static std::shared_ptr<BackgroundTool> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _widgetUpdate();

        std::shared_ptr<GlobalViewModel> _model;
        GlobalViewOptions _options;

        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::shared_ptr<feather_tk::ComboBox> _comboBox;
        std::shared_ptr<feather_tk::ColorWidget> _solidColorWidget;
        std::shared_ptr<feather_tk::ColorWidget> _checkersColor0Widget;
        std::shared_ptr<feather_tk::ColorWidget> _checkersColor1Widget;
        std::shared_ptr<feather_tk::IntEditSlider> _checkersSizeSlider;

        std::shared_ptr<feather_tk::ValueObserver<GlobalViewOptions> > _optionsObserver;
    };
}

