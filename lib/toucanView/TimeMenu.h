// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/SelectionModel.h>

#include <feather-tk/ui/Menu.h>

#include <opentimelineio/version.h>

#include <map>

namespace toucan
{
    class App;
    class File;

    //! Time menu.
    class TimeMenu : public feather_tk::Menu
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimeMenu();

        //! Create a new menu.
        static std::shared_ptr<TimeMenu> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the actions.
        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& getActions() const;

    private:
        void _menuUpdate();

        std::shared_ptr<File> _file;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::TimeRange _inOutRange;
        bool _selection = false;

        std::map<std::string, std::shared_ptr<feather_tk::Action> > _actions;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::TimeRange> > _timeRangeObserver;
        std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::TimeRange> > _inOutRangeObserver;
        std::shared_ptr<feather_tk::ListObserver<SelectionItem> > _selectionObserver;
    };
}

