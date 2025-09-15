// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/FFmpegRead.h>
#include <toucanRender/ImageNode.h>
#include <toucanRender/MemoryMap.h>

#include <lunasvg/lunasvg.h>

#include <OpenImageIO/filesystem.h>

#include <filesystem>

namespace toucan
{
    //! Base class for read nodes.
    class IReadNode : public IImageNode
    {
    public:
        IReadNode(const std::string& name);

        virtual ~IReadNode() = 0;

        const OIIO::ImageSpec& getSpec() const;

        const OTIO_NS::TimeRange& getTimeRange() const;

    protected:
        OIIO::ImageSpec _spec;
        OTIO_NS::TimeRange _timeRange;
    };

    //! Image read node.
    class ImageReadNode : public IReadNode
    {
    public:
        ImageReadNode(
            const std::filesystem::path&,
            const MemoryReference& = {});

        virtual ~ImageReadNode();

        std::string getLabel() const override;

        OIIO::ImageBuf exec() override;

        static std::vector<std::string> getExtensions();

    private:
        std::filesystem::path _path;
        std::shared_ptr<OIIO::Filesystem::IOMemReader> _memoryReader;
        std::unique_ptr<OIIO::ImageInput> _input;
    };

    //! Image sequence read node.
    class SequenceReadNode : public IReadNode
    {
    public:
        SequenceReadNode(
            const std::string& base,
            const std::string& namePrefix,
            const std::string& nameSuffix,
            int startFrame,
            int frameStep,
            double rate,
            int frameZeroPadding,
            const MemoryReferences& = {});

        virtual ~SequenceReadNode();

        std::string getLabel() const override;

        OIIO::ImageBuf exec() override;

        static std::vector<std::string> getExtensions();

    private:
        std::string _base;
        std::string _namePrefix;
        std::string _nameSuffix;
        int _startFrame = 1;
        int _frameStep = 1;
        double _rate = 1.0;
        int _frameZeroPadding = 0;
        MemoryReferences _memoryReferences;
    };

    //! SVG read node.
    class SVGReadNode : public IReadNode
    {
    public:
        SVGReadNode(
            const std::filesystem::path&,
            const MemoryReference& = {});

        virtual ~SVGReadNode();

        std::string getLabel() const override;

        OIIO::ImageBuf exec() override;

        static std::vector<std::string> getExtensions();

    private:
        std::filesystem::path _path;
        std::unique_ptr<lunasvg::Document> _svg;
    };

    //! Movie read node.
    class MovieReadNode : public IReadNode
    {
    public:
        MovieReadNode(
            const std::filesystem::path&,
            const MemoryReference& = {});

        virtual ~MovieReadNode();

        std::string getLabel() const override;

        OIIO::ImageBuf exec() override;

        static std::vector<std::string> getExtensions();

    private:
        std::filesystem::path _path;
        std::shared_ptr<OIIO::Filesystem::IOMemReader> _memoryReader;
        std::unique_ptr<ffmpeg::Read> _ffRead;
    };

    //! Create a read node.
    std::shared_ptr<IReadNode> createReadNode(
        const std::filesystem::path&,
        const MemoryReference& = {});

    //! Create a read node.
    std::shared_ptr<IReadNode> createReadNode(
        const std::string& base,
        const std::string& namePrefix,
        const std::string& nameSuffix,
        int startFrame,
        int frameStep,
        double rate,
        int frameZerPadding,
        const MemoryReferences& = {});

    //! Is the extension in the list?
    bool hasExtension(
        const std::string& extension,
        const std::vector<std::string>& extensions);
}
