// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <ftk/Ui/Action.h>
#include <ftk/Ui/RowLayout.h>

namespace toucan
{
    class App;
    class MainWindow;

    //! Tool bar.
    class ToolBar : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<ftk::Action> >&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ToolBar();

        //! Create a new tool bar.
        static std::shared_ptr<ToolBar> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<MainWindow>&,
            const std::map<std::string, std::shared_ptr<ftk::Action> >&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        std::shared_ptr<ftk::HorizontalLayout> _layout;
        std::map<std::string, std::shared_ptr<ftk::IWidget> > _toolBars;
    };
}

