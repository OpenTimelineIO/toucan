// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Read.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

#include <sstream>

namespace toucan
{
    IReadNode::IReadNode(
        const OTIO_NS::MediaReference* ref,
        const std::string& name,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode(name, inputs),
        _ref(ref)
    {}

    IReadNode::~IReadNode()
    {}

    const OTIO_NS::MediaReference* IReadNode::getRef() const
    {
        return _ref;
    }

    const OIIO::ImageSpec& IReadNode::getSpec() const
    {
        return _spec;
    }

    const OTIO_NS::TimeRange& IReadNode::getTimeRange() const
    {
        return _timeRange;
    }

    ImageReadNode::ImageReadNode(
        const std::filesystem::path& path,
        const OTIO_NS::MediaReference* ref,
        const MemoryReference& memoryReference,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IReadNode(ref, "ImageRead", inputs),
        _path(path),
        _memoryReader(getMemoryReader(memoryReference))
    {
        _input = OIIO::ImageInput::open(_path.string(), nullptr, _memoryReader.get());
        if (!_input)
        {
            std::stringstream ss;
            ss << "Cannot open file: " << _path.string();
            throw std::runtime_error(ss.str());
        }
        _spec = _input->spec();
    }

    ImageReadNode::~ImageReadNode()
    {
        if (_input)
        {
            _input->close();
        }
    }

    std::string ImageReadNode::getLabel() const
    {
        std::stringstream ss;
        ss << "Read: " << _path.filename().string();
        return ss.str();
    }

    OIIO::ImageBuf ImageReadNode::exec()
    {
        OIIO::ImageBuf out;

        // Read the image.
        auto pixels = std::unique_ptr<unsigned char[]>(
            new unsigned char[_spec.width * _spec.height * _spec.nchannels * _spec.channel_bytes()]);
        _input->read_image(
            0,
            0,
            0,
            _spec.nchannels,
            _spec.format,
            &pixels[0]);

        OIIO::ImageBuf buf(
            OIIO::ImageSpec(_spec.width, _spec.height, _spec.nchannels, _spec.format),
            pixels.get(),
            _spec.nchannels * _spec.channel_bytes(),
            _spec.width * _spec.nchannels * _spec.channel_bytes(),
            0);

        const auto& spec = buf.spec();
        if (3 == spec.nchannels)
        {
            // Add an alpha channel.
            const int channelOrder[] = { 0, 1, 2, -1 };
            const float channelValues[] = { 0, 0, 0, 1.0 };
            const std::string channelNames[] = { "", "", "", "A" };
            out = OIIO::ImageBufAlgo::channels(buf, 4, channelOrder, channelValues, channelNames);
        }
        else
        {
            OIIO::ImageBufAlgo::copy(out, buf);
        }

        return out;
    }

    std::vector<std::string> ImageReadNode::getExtensions()
    {
        return std::vector<std::string>({ ".exr", ".tif", ".tiff", ".jpg", ".jpeg", ".png" });
    }

    bool ImageReadNode::hasExtension(const std::string& value)
    {
        const std::vector<std::string> extensions = getExtensions();
        const auto i = std::find(extensions.begin(), extensions.end(), toLower(value));
        return i != extensions.end();
    }

    SequenceReadNode::SequenceReadNode(
        const std::string& base,
        const std::string& namePrefix,
        const std::string& nameSuffix,
        int startFrame,
        int frameStep,
        double rate,
        int frameZeroPadding,
        const OTIO_NS::MediaReference* ref,
        const MemoryReferences& memoryReferences,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IReadNode(ref, "SequenceRead", inputs),
        _base(base),
        _namePrefix(namePrefix),
        _nameSuffix(nameSuffix),
        _startFrame(startFrame),
        _frameStep(frameStep),
        _rate(rate),
        _frameZeroPadding(frameZeroPadding),
        _memoryReferences(memoryReferences)
    {
        // Get information from the first frame.
        const std::string url = getSequenceFrame(
            _base,
            _namePrefix,
            _startFrame,
            _frameZeroPadding,
            _nameSuffix);
        std::unique_ptr<OIIO::Filesystem::IOMemReader> memoryReader;
        const auto i = _memoryReferences.find(url);
        if (i != _memoryReferences.end() && i->second.isValid())
        {
            memoryReader = getMemoryReader(i->second);
        }
        if (auto input = OIIO::ImageInput::open(url, nullptr, memoryReader.get()))
        {
            _spec = input->spec();
        }
        _timeRange = OTIO_NS::TimeRange(
            OTIO_NS::RationalTime(_startFrame, _rate),
            OTIO_NS::RationalTime(1.0, _rate));
    }

    SequenceReadNode::~SequenceReadNode()
    {}

    std::string SequenceReadNode::getLabel() const
    {
        std::stringstream ss;
        ss << "Read: " << getSequenceFrame(
            _base,
            _namePrefix,
            _startFrame,
            _frameZeroPadding,
            _nameSuffix);
        return ss.str();
    }

    OIIO::ImageBuf SequenceReadNode::exec()
    {
        OIIO::ImageBuf out;

        // Get the time.
        OTIO_NS::RationalTime offsetTime = _time;
        if (!_timeOffset.is_invalid_time())
        {
            offsetTime -= _timeOffset;
        }

        // Open the sequence file.
        const std::string url = getSequenceFrame(
            _base,
            _namePrefix,
            offsetTime.floor().to_frames(),
            _frameZeroPadding,
            _nameSuffix);
        std::unique_ptr<OIIO::Filesystem::IOMemReader> memoryReader;
        const auto i = _memoryReferences.find(url);
        if (i != _memoryReferences.end() && i->second.isValid())
        {
            memoryReader = getMemoryReader(i->second);
        }
        if (auto input = OIIO::ImageInput::open(url, nullptr, memoryReader.get()))
        {
            // Read the image.
            const auto& spec = input->spec();
            auto pixels = std::unique_ptr<unsigned char[]>(
                new unsigned char[spec.width * spec.height * spec.nchannels * spec.channel_bytes()]);
            input->read_image(
                0,
                0,
                0,
                spec.nchannels,
                spec.format,
                &pixels[0]);

            OIIO::ImageBuf buf(
                OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, spec.format),
                pixels.get(),
                spec.nchannels * spec.channel_bytes(),
                spec.width * spec.nchannels * spec.channel_bytes(),
                0);
            if (3 == spec.nchannels)
            {
                // Add an alpha channel.
                const int channelOrder[] = { 0, 1, 2, -1 };
                const float channelValues[] = { 0, 0, 0, 1.0 };
                const std::string channelNames[] = { "", "", "", "A" };
                out = OIIO::ImageBufAlgo::channels(buf, 4, channelOrder, channelValues, channelNames);
            }
            else
            {
                OIIO::ImageBufAlgo::copy(out, buf);
            }
        }

        return out;
    }

    std::vector<std::string> SequenceReadNode::getExtensions()
    {
        return std::vector<std::string>({ ".exr", ".tif", ".png" });
    }

    bool SequenceReadNode::hasExtension(const std::string& value)
    {
        const std::vector<std::string> extensions = getExtensions();
        const auto i = std::find(extensions.begin(), extensions.end(), toLower(value));
        return i != extensions.end();
    }

    MovieReadNode::MovieReadNode(
        const std::filesystem::path& path,
        const OTIO_NS::MediaReference* ref,
        const MemoryReference& memoryReference,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IReadNode(ref, "MovieReadNode", inputs),
        _path(path),
        _memoryReader(getMemoryReader(memoryReference))
    {
        _ffRead = std::make_unique<ffmpeg::Read>(path, memoryReference);
        _spec = _ffRead->getSpec();
        _timeRange = _ffRead->getTimeRange();
    }

    MovieReadNode::~MovieReadNode()
    {}

    std::string MovieReadNode::getLabel() const
    {
        std::stringstream ss;
        ss << "Read: " << _path.filename().string();
        return ss.str();
    }

    OIIO::ImageBuf MovieReadNode::exec()
    {
        OIIO::ImageBuf out;

        // Get the time.
        OTIO_NS::RationalTime offsetTime = _time;
        if (!_timeOffset.is_invalid_time())
        {
            offsetTime -= _timeOffset;
        }

        // Read the image.
        out = _ffRead->getImage(offsetTime);

        const auto& spec = out.spec();
        if (3 == spec.nchannels)
        {
            // Add an alpha channel.
            const int channelOrder[] = { 0, 1, 2, -1 };
            const float channelValues[] = { 0, 0, 0, 1.0 };
            const std::string channelNames[] = { "", "", "", "A" };
            out = OIIO::ImageBufAlgo::channels(out, 4, channelOrder, channelValues, channelNames);
        }

        return out;
    }

    std::vector<std::string> MovieReadNode::getExtensions()
    {
        return std::vector<std::string>({ ".mov", ".mp4", ".m4v", ".y4m" });
    }

    bool MovieReadNode::hasExtension(const std::string& value)
    {
        const std::vector<std::string> extensions = getExtensions();
        const auto i = std::find(extensions.begin(), extensions.end(), toLower(value));
        return i != extensions.end();
    }
}
