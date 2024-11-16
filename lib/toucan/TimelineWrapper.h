// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "MemoryMap.h"

#include <opentimelineio/timeline.h>

#include <filesystem>
#include <map>
#include <memory>

namespace toucan
{
    //! Timeline wrapper that supports .otiod and .otioz files.
    class TimelineWrapper : public std::enable_shared_from_this<TimelineWrapper>
    {
    public:
        TimelineWrapper(const std::filesystem::path&);

        ~TimelineWrapper();

        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& getTimeline() const;

        const OTIO_NS::TimeRange& getTimeRange() const;

        std::string getMediaPath(const std::string& url) const;

        const MemoryReferences& getMemoryReferences() const;
        MemoryReference getMemoryReference(const std::string& url) const;

    private:
        std::filesystem::path _path;
        std::filesystem::path _tmpPath;
        std::unique_ptr<MemoryMap> _memoryMap;
        MemoryReferences _memoryReferences;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> _timeline;
        OTIO_NS::TimeRange _timeRange;
    };
}
