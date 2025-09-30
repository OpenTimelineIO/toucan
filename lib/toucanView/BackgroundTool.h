// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/FilesModel.h>
#include <toucanView/ViewModel.h>
#include <toucanView/IToolWidget.h>

#include <ftk/Ui/ColorWidget.h>
#include <ftk/Ui/ComboBox.h>
#include <ftk/Ui/IntEditSlider.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/ScrollWidget.h>

namespace toucan
{
    class File;

    //! Background tool.
    class BackgroundTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~BackgroundTool();

        //! Create a new tool.
        static std::shared_ptr<BackgroundTool> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        void _widgetUpdate();

        std::shared_ptr<GlobalViewModel> _model;
        GlobalViewOptions _options;

        std::shared_ptr<ftk::ScrollWidget> _scrollWidget;
        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<ftk::ComboBox> _comboBox;
        std::shared_ptr<ftk::ColorWidget> _solidColorWidget;
        std::shared_ptr<ftk::ColorWidget> _checkersColor0Widget;
        std::shared_ptr<ftk::ColorWidget> _checkersColor1Widget;
        std::shared_ptr<ftk::IntEditSlider> _checkersSizeSlider;

        std::shared_ptr<ftk::ValueObserver<GlobalViewOptions> > _optionsObserver;
    };
}

