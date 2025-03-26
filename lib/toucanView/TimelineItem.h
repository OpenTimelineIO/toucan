// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IItem.h>
#include <toucanView/SelectionModel.h>

#include <dtk/core/ObservableList.h>

namespace toucan
{
    class SelectionModel;

    //! Timeline item.
    class TimelineItem : public IItem
    {
    protected:
        void _init(
            const std::shared_ptr<dtk::Context>&,
            const ItemData&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimelineItem();

        //! Create a new item.
        static std::shared_ptr<TimelineItem> create(
            const std::shared_ptr<dtk::Context>&,
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

        void setGeometry(const dtk::Box2I&) override;
        void tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const dtk::TickEvent&) override;
        void sizeHintEvent(const dtk::SizeHintEvent&) override;
        void drawEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;
        void drawOverlayEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;
        void mouseMoveEvent(dtk::MouseMoveEvent&) override;
        void mousePressEvent(dtk::MouseClickEvent&) override;
        void mouseReleaseEvent(dtk::MouseClickEvent&) override;

    protected:
        void _timeUnitsUpdate() override;

    private:
        dtk::Size2I _getLabelMaxSize(
            const std::shared_ptr<dtk::FontSystem>&) const;
        void _getTimeTicks(
            const std::shared_ptr<dtk::FontSystem>&,
            double& seconds,
            int& tick);
        void _drawTimeTicks(
            const dtk::Box2I&,
            const dtk::DrawEvent&);
        void _drawTimeLabels(
            const dtk::Box2I&,
            const dtk::DrawEvent&);

        void _select(
            const std::shared_ptr<dtk::IWidget>&,
            const dtk::V2I&,
            std::shared_ptr<IItem>&);
        void _select(
            const std::shared_ptr<dtk::IWidget>&,
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
        std::shared_ptr<dtk::LRUCache<std::string, std::shared_ptr<dtk::Image> > > _thumbnailCache;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int margin = 0;
            int border = 0;
            int handle = 0;
            int thumbnailHeight = 0;
            dtk::FontInfo fontInfo;
            dtk::FontMetrics fontMetrics;
            dtk::V2I scrollPos;
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

        std::shared_ptr<dtk::ListObserver<SelectionItem > > _selectionObserver;
        std::shared_ptr<dtk::ValueObserver<bool> > _thumbnailsObserver;
    };
}
