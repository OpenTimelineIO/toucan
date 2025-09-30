// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <ftk/Ui/Action.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/ToolButton.h>
#include <ftk/Core/ObservableList.h>

namespace toucan
{
    class App;
    class File;

    //! File tool bar.
    class FileToolBar : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::map<std::string, std::shared_ptr<ftk::Action> >&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~FileToolBar();

        //! Create a new tool bar.
        static std::shared_ptr<FileToolBar> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::map<std::string, std::shared_ptr<ftk::Action> >&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        void _widgetUpdate();

        size_t _filesSize = 0;

        std::shared_ptr<ftk::HorizontalLayout> _layout;
        std::map<std::string, std::shared_ptr<ftk::ToolButton> > _buttons;

        std::shared_ptr<ftk::ListObserver<std::shared_ptr<File> > > _filesObserver;
    };
}

