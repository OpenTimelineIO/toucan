// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IItem.h>
#include <toucanView/SelectionModel.h>

#include <feather-tk/core/ObservableList.h>

namespace toucan
{
    class SelectionModel;

    //! Timeline item.
    class TimelineItem : public IItem
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const ItemData&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimelineItem();

        //! Create a new item.
        static std::shared_ptr<TimelineItem> create(
            const std::shared_ptr<feather_tk::Context>&,
            const ItemData&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the current time.
        const OTIO_NS::RationalTime& getCurrentTime() const;

        //! Set the current time.
        void setCurrentTime(const OTIO_NS::RationalTime&);

        //! Set the current time callback.
        void setCurrentTimeCallback(const std::function<void(const OTIO_NS::RationalTime&)>&);

        //! Set the in/out range.
        void setInOutRange(const OTIO_NS::TimeRange&);

        void setGeometry(const feather_tk::Box2I&) override;
        void tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const feather_tk::TickEvent&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;
        void drawOverlayEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;
        void mouseMoveEvent(feather_tk::MouseMoveEvent&) override;
        void mousePressEvent(feather_tk::MouseClickEvent&) override;
        void mouseReleaseEvent(feather_tk::MouseClickEvent&) override;

    protected:
        void _timeUnitsUpdate() override;

    private:
        feather_tk::Size2I _getLabelMaxSize(
            const std::shared_ptr<feather_tk::FontSystem>&) const;
        void _getTimeTicks(
            const std::shared_ptr<feather_tk::FontSystem>&,
            double& seconds,
            int& tick);
        void _drawTimeTicks(
            const feather_tk::Box2I&,
            const feather_tk::DrawEvent&);
        void _drawTimeLabels(
            const feather_tk::Box2I&,
            const feather_tk::DrawEvent&);

        void _select(
            const std::shared_ptr<feather_tk::IWidget>&,
            const feather_tk::V2I&,
            std::shared_ptr<IItem>&);
        void _select(
            const std::shared_ptr<feather_tk::IWidget>&,
            const std::vector<SelectionItem>&);

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> _timeline;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _currentTime = OTIO_NS::RationalTime(-1.0, -1.0);
        std::function<void(const OTIO_NS::RationalTime&)> _currentTimeCallback;
        OTIO_NS::TimeRange _inOutRange;
        std::shared_ptr<SelectionModel> _selectionModel;
        bool _thumbnails = true;
        std::shared_ptr<ThumbnailGenerator> _thumbnailGenerator;
        std::list<ThumbnailRequest> _thumbnailRequests;
        std::shared_ptr<feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > > _thumbnailCache;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int margin = 0;
            int border = 0;
            int handle = 0;
            int thumbnailHeight = 0;
            feather_tk::FontInfo fontInfo;
            feather_tk::FontMetrics fontMetrics;
            feather_tk::V2I scrollPos;
        };
        SizeData _size;

        enum MouseMode
        {
            None,
            CurrentTime,
            Select
        };
        struct MouseData
        {
            MouseMode mode = MouseMode::None;
        };
        MouseData _mouse;

        std::shared_ptr<feather_tk::ListObserver<SelectionItem > > _selectionObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _thumbnailsObserver;
    };
}
