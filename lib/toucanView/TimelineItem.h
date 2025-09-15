// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/IItem.h>
#include <toucanView/SelectionModel.h>

#include <feather-tk/core/ObservableList.h>

namespace toucan
{
    class StackItem;

    //! Timeline item.
    class TimelineItem : public IItem
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const ItemData&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~TimelineItem();

        //! Create a new item.
        static std::shared_ptr<TimelineItem> create(
            const std::shared_ptr<ftk::Context>&,
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

        void setGeometry(const ftk::Box2I&) override;
        void sizeHintEvent(const ftk::SizeHintEvent&) override;
        void drawOverlayEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;
        void mouseMoveEvent(ftk::MouseMoveEvent&) override;
        void mousePressEvent(ftk::MouseClickEvent&) override;
        void mouseReleaseEvent(ftk::MouseClickEvent&) override;

    protected:
        void _timeUnitsUpdate() override;

    private:
        ftk::Size2I _getLabelMaxSize(
            const std::shared_ptr<ftk::FontSystem>&) const;
        void _getTimeTicks(
            const std::shared_ptr<ftk::FontSystem>&,
            double& seconds,
            int& tick);
        void _drawTimeTicks(
            const ftk::Box2I&,
            const ftk::DrawEvent&);
        void _drawTimeLabels(
            const ftk::Box2I&,
            const ftk::DrawEvent&);

        void _select(
            const std::shared_ptr<ftk::IWidget>&,
            const ftk::V2I&,
            std::shared_ptr<IItem>&);
        void _select(
            const std::shared_ptr<ftk::IWidget>&,
            const std::vector<SelectionItem>&);

        const OTIO_NS::Timeline* _timeline = nullptr;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _currentTime = OTIO_NS::RationalTime(-1.0, -1.0);
        std::function<void(const OTIO_NS::RationalTime&)> _currentTimeCallback;
        OTIO_NS::TimeRange _inOutRange;
        std::shared_ptr<SelectionModel> _selectionModel;
        std::shared_ptr<StackItem> _stackItem;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int margin = 0;
            int border = 0;
            int handle = 0;
            int thumbnailHeight = 0;
            ftk::FontInfo fontInfo;
            ftk::FontMetrics fontMetrics;
            ftk::V2I scrollPos;
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

        std::shared_ptr<ftk::ListObserver<SelectionItem > > _selectionObserver;
    };
}
