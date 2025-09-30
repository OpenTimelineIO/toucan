// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/WindowModel.h>

#include <ftk/Ui/Action.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/ToolButton.h>
#include <ftk/Core/ObservableList.h>

namespace toucan
{
    class App;
    class File;
    class MainWindow;

    //! Window tool bar.
    class WindowToolBar : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<ftk::Action> >&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~WindowToolBar();

        //! Create a new tool bar.
        static std::shared_ptr<WindowToolBar> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<ftk::Action> >&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        std::shared_ptr<ftk::HorizontalLayout> _layout;
        std::map<std::string, std::shared_ptr<ftk::ToolButton> > _buttons;

        std::shared_ptr<ftk::ValueObserver<bool> > _frameViewObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _fullScreenObserver;
        std::shared_ptr<ftk::MapObserver<WindowComponent, bool> > _componentObserver;
    };
}

