// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/PlaybackModel.h>

#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/Label.h>

namespace toucan
{
    class App;
    class File;

    //! HUD widget.
    class HUDWidget : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~HUDWidget();

        //! Create a new widget.
        static std::shared_ptr<HUDWidget> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void setGeometry(const ftk::Box2I&) override;

    private:
        void _widgetUpdate();
        
        std::shared_ptr<File> _file;
        OTIO_NS::RationalTime _currentTime;
        OTIO_NS::TimeRange _timeRange;

        std::shared_ptr<ftk::VerticalLayout> _layout;
        std::map<std::string, std::shared_ptr<ftk::Label> > _labels;

        std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
        std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > _timeRangeObserver;
    };
}

