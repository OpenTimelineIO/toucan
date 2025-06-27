// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/FilesModel.h>
#include <toucanView/IToolWidget.h>

#include <feather-tk/ui/CheckBox.h>
#include <feather-tk/ui/ComboBox.h>
#include <feather-tk/ui/GridLayout.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/core/ObservableList.h>

namespace toucan
{
    class File;

    //! Compare widget.
    class CompareWidget : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~CompareWidget();

        //! Create a new widget.
        static std::shared_ptr<CompareWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setCurrent(bool);
        void setCurrentCallback(const std::function<void(void)>&);

        void setB(bool);
        void setBCallback(const std::function<void(bool)>&);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        bool _current = false;
        bool _b = false;

        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::shared_ptr<feather_tk::ToolButton> _button;
        std::shared_ptr<feather_tk::ToolButton> _bButton;
    };

    //! Compare tool.
    class CompareTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~CompareTool();

        //! Create a new tool.
        static std::shared_ptr<CompareTool> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        void _widgetUpdate();
        void _indexUpdate();

        std::shared_ptr<FilesModel> _filesModel;
        std::vector<std::shared_ptr<File> > _files;
        int _currentIndex = -1;
        int _bIndex = -1;

        std::shared_ptr<feather_tk::VerticalLayout> _layout;
        std::shared_ptr<feather_tk::ScrollWidget> _scrollWidget;
        std::shared_ptr<feather_tk::VerticalLayout> _widgetLayout;
        std::vector<std::shared_ptr<CompareWidget> > _widgets;
        std::shared_ptr<feather_tk::GridLayout> _bottomLayout;
        std::shared_ptr<feather_tk::ComboBox> _modeComboBox;
        std::shared_ptr<feather_tk::CheckBox> _startTimeCheckBox;
        std::shared_ptr<feather_tk::CheckBox> _resizeCheckBox;

        std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<File> > > _filesObserver;
        std::shared_ptr<feather_tk::ValueObserver<int> > _fileIndexObserver;
        std::shared_ptr<feather_tk::ValueObserver<int> > _bIndexObserver;
        std::shared_ptr<feather_tk::ValueObserver<CompareOptions> > _compareOptionsObserver;
    };
}

