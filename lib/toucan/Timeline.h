// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>

#include <filesystem>
#include <memory>

namespace toucan
{
    class Timeline : public std::enable_shared_from_this<Timeline>
    {
    public:
        Timeline(const std::filesystem::path&);

        ~Timeline();

        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& otio() const;

        const OTIO_NS::TimeRange& getTimeRange() const;

        std::filesystem::path getMediaPath(const std::string& url) const;

    private:
        std::filesystem::path _path;
        std::filesystem::path _tmpPath;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> _timeline;
        OTIO_NS::TimeRange _timeRange;
    };
}
