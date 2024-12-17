// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageNode.h>
#include <toucanRender/TimelineWrapper.h>

#include <dtk/core/Context.h>
#include <dtk/core/LRUCache.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/track.h>
#include <opentimelineio/transition.h>

#include <filesystem>
#include <memory>

namespace toucan
{
    class ReadNode;

    //! Create image graphs from a timeline.
    class ImageGraph : public std::enable_shared_from_this<ImageGraph>
    {
    public:
        ImageGraph(
            const std::shared_ptr<dtk::Context>&,
            const std::filesystem::path&,
            const std::shared_ptr<TimelineWrapper>&);

        ~ImageGraph();

        //! Get the timeline image size.
        const IMATH_NAMESPACE::V2i& getImageSize() const;

        //! Get the timeline image channels.
        int getImageChannels() const;

        //! Get the timeline image data type.
        const std::string& getImageDataType() const;

        //! Get an image graph for the given time.
        std::shared_ptr<IImageNode> exec(
            const std::shared_ptr<ImageEffectHost>&,
            const OTIO_NS::RationalTime&);

    private:
        std::shared_ptr<IImageNode> _track(
            const std::shared_ptr<ImageEffectHost>&,
            const OTIO_NS::RationalTime&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>&);

        std::shared_ptr<IImageNode> _item(
            const std::shared_ptr<ImageEffectHost>&,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::RationalTime&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>&);

        std::shared_ptr<IImageNode> _effects(
            const std::shared_ptr<ImageEffectHost>&,
            const std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Effect> >&,
            const std::shared_ptr<IImageNode>&);

        std::weak_ptr<dtk::Context> _context;
        std::filesystem::path _path;
        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        OTIO_NS::TimeRange _timeRange;
        IMATH_NAMESPACE::V2i _imageSize = IMATH_NAMESPACE::V2i(0, 0);
        int _imageChannels = 0;
        std::string _imageDataType;
        dtk::LRUCache<OTIO_NS::MediaReference*, std::shared_ptr<ReadNode> > _loadCache;
    };
}
