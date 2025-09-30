// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/FilesModel.h>
#include <toucanView/IToolWidget.h>

#include <ftk/Ui/CheckBox.h>
#include <ftk/Ui/ComboBox.h>
#include <ftk/Ui/GridLayout.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/ScrollWidget.h>
#include <ftk/Ui/ToolButton.h>
#include <ftk/Core/ObservableList.h>

namespace toucan
{
    class File;

    //! Compare widget.
    class CompareWidget : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~CompareWidget();

        //! Create a new widget.
        static std::shared_ptr<CompareWidget> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setCurrent(bool);
        void setCurrentCallback(const std::function<void(void)>&);

        void setB(bool);
        void setBCallback(const std::function<void(bool)>&);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        bool _current = false;
        bool _b = false;

        std::shared_ptr<ftk::HorizontalLayout> _layout;
        std::shared_ptr<ftk::ToolButton> _button;
        std::shared_ptr<ftk::ToolButton> _bButton;
    };

    //! Compare tool.
    class CompareTool : public IToolWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~CompareTool();

        //! Create a new tool.
        static std::shared_ptr<CompareTool> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        void _widgetUpdate();
        void _indexUpdate();

        std::shared_ptr<FilesModel> _filesModel;
        std::vector<std::shared_ptr<File> > _files;
        int _currentIndex = -1;
        int _bIndex = -1;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::shared_ptr<ftk::ScrollWidget> _scrollWidget;
        std::shared_ptr<ftk::VerticalLayout> _widgetLayout;
        std::vector<std::shared_ptr<CompareWidget> > _widgets;
        std::shared_ptr<ftk::GridLayout> _bottomLayout;
        std::shared_ptr<ftk::ComboBox> _modeComboBox;
        std::shared_ptr<ftk::CheckBox> _startTimeCheckBox;
        std::shared_ptr<ftk::CheckBox> _resizeCheckBox;

        std::shared_ptr<ftk::ListObserver<std::shared_ptr<File> > > _filesObserver;
        std::shared_ptr<ftk::ValueObserver<int> > _fileIndexObserver;
        std::shared_ptr<ftk::ValueObserver<int> > _bIndexObserver;
        std::shared_ptr<ftk::ValueObserver<CompareOptions> > _compareOptionsObserver;
    };
}

