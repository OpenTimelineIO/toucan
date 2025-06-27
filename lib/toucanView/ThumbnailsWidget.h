// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/ThumbnailGenerator.h>
#include <toucanView/TimeLayout.h>

#include <opentimelineio/clip.h>

namespace toucan
{
    //! Timeline thumbnails widget.
    class ThumbnailsWidget : public ITimeWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::MediaReference>&,
            const std::shared_ptr<ThumbnailGenerator>&,
            const std::shared_ptr<feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > >&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~ThumbnailsWidget();

        //! Create a new widget.
        static std::shared_ptr<ThumbnailsWidget> create(
            const std::shared_ptr<feather_tk::Context>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::MediaReference>&,
            const std::shared_ptr<ThumbnailGenerator>&,
            const std::shared_ptr<feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > >&,
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        void tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const feather_tk::TickEvent&) override;
        void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
        void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

    private:
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::MediaReference> _ref;
        float _thumbnailAspect = 0.F;
        std::shared_ptr<ThumbnailGenerator> _thumbnailGenerator;
        std::shared_ptr<feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > > _thumbnailCache;
        std::future<float> _thumbnailAspectRequest;
        std::list<ThumbnailRequest> _thumbnailRequests;

        struct SizeData
        {
            bool init = true;
            float displayScale = 0.F;
            int thumbnailHeight = 0;
        };
        SizeData _size;
    };
}
