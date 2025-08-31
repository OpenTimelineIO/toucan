// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/ViewModel.h>

#include <feather-tk/ui/IWidget.h>

namespace toucan
{
    class App;
    class File;
    class HUDWidget;
    class Viewport;

    //! Timeline file tab.
    class FileTab : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~FileTab();

        //! Create a new tab.
        static std::shared_ptr<FileTab> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;

    private:
        std::shared_ptr<Viewport> _viewport;
        std::shared_ptr<HUDWidget> _hudWidget;
        std::shared_ptr<ftk::ValueObserver<GlobalViewOptions> > _viewOptionsObserver;
    };
}

