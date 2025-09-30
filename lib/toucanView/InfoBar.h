// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <ftk/Ui/Action.h>
#include <ftk/Ui/RowLayout.h>
#include <ftk/Ui/Label.h>

#include <ftk/Core/ObservableList.h>

namespace toucan
{
    class App;
    class File;

    //! Information tool bar.
    class InfoBar : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~InfoBar();

        //! Create a new tool bar.
        static std::shared_ptr<InfoBar> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        std::shared_ptr<ftk::HorizontalLayout> _layout;
        std::shared_ptr<ftk::Label> _label;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
    };
}

