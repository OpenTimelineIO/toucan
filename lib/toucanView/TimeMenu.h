// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/SelectionModel.h>

#include <ftk/Ui/Menu.h>

#include <opentimelineio/version.h>

#include <map>

namespace toucan
{
    class App;
    class File;

    //! Time menu.
    class TimeMenu : public ftk::Menu
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimeMenu();

        //! Create a new menu.
        static std::shared_ptr<TimeMenu> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the actions.
        const std::map<std::string, std::shared_ptr<ftk::Action> >& getActions() const;

    private:
        void _menuUpdate();

        std::shared_ptr<File> _file;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::TimeRange _inOutRange;
        bool _selection = false;

        std::map<std::string, std::shared_ptr<ftk::Action> > _actions;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _fileObserver;
        std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > _timeRangeObserver;
        std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > _inOutRangeObserver;
        std::shared_ptr<ftk::ListObserver<SelectionItem> > _selectionObserver;
    };
}

