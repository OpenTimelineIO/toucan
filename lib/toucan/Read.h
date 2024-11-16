// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "MemoryMap.h"

#include <toucan/FFmpegRead.h>
#include <toucan/ImageNode.h>

#include <OpenImageIO/filesystem.h>

#include <filesystem>

namespace toucan
{
    //! Read node.
    class ReadNode : public IImageNode
    {
    public:
        ReadNode(
            const std::filesystem::path&,
            const MemoryReference& = {},
            const std::vector<std::shared_ptr<IImageNode> >& = {});

        virtual ~ReadNode();

        const OIIO::ImageSpec& getSpec() const;

        const OTIO_NS::TimeRange& getTimeRange() const;
        
        std::string getLabel() const override;

        OIIO::ImageBuf exec() override;

    private:
        std::filesystem::path _path;
        std::unique_ptr<FFmpegRead> _ffRead;
        std::shared_ptr<OIIO::Filesystem::IOMemReader> _memoryReader;
        std::unique_ptr<OIIO::ImageInput> _input;
        OIIO::ImageSpec _spec;
        OTIO_NS::TimeRange _timeRange;
    };

    //! Sequence read node.
    class SequenceReadNode : public IImageNode
    {
    public:
        SequenceReadNode(
            const std::string& base,
            const std::string& namePrefix,
            const std::string& nameSuffix,
            int startFrame,
            int frameStep,
            double rate,
            int frameZerNodeadding,
            const MemoryReferences& = {},
            const std::vector<std::shared_ptr<IImageNode> >& = {});

        virtual ~SequenceReadNode();

        const OIIO::ImageSpec& getSpec() const;

        std::string getLabel() const override;

        OIIO::ImageBuf exec() override;

    private:
        std::string _base;
        std::string _namePrefix;
        std::string _nameSuffix;
        int _startFrame = 1;
        int _frameStep = 1;
        double _rate = 1.0;
        int _frameZeroPadding = 0;
        MemoryReferences _memoryReferences;
        OIIO::ImageSpec _spec;
    };
}
