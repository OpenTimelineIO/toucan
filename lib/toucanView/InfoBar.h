// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/Action.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/Label.h>

#include <feather-tk/core/ObservableList.h>

namespace toucan
{
    class App;
    class File;

    //! Information tool bar.
    class InfoBar : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~InfoBar();

        //! Create a new tool bar.
        static std::shared_ptr<InfoBar> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const feather_tk::Box2I&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

    private:
        std::shared_ptr<feather_tk::HorizontalLayout> _layout;
        std::shared_ptr<feather_tk::Label> _label;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
    };
}

