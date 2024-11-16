// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Read.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

#include <sstream>

namespace toucan
{
    ReadNode::ReadNode(
        const std::filesystem::path& path,
        const MemoryReference& memoryReference,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode("Read", inputs),
        _path(path),
        _memoryReader(getMemoryReader(memoryReference))
    {
        try
        {
            _ffRead = std::make_unique<FFmpegRead>(path, memoryReference);
            _spec = _ffRead->getSpec();
            _timeRange = _ffRead->getTimeRange();
        }
        catch (const std::exception&)
        {}
        if (!_ffRead)
        {
            _input = OIIO::ImageInput::open(_path.string(), nullptr, _memoryReader.get());
            if (_input)
            {
                _spec = _input->spec();
            }
        }
    }

    ReadNode::~ReadNode()
    {
        if (_input)
        {
            _input->close();
        }
    }

    const OIIO::ImageSpec& ReadNode::getSpec() const
    {
        return _spec;
    }

    const OTIO_NS::TimeRange& ReadNode::getTimeRange() const
    {
        return _timeRange;
    }

    std::string ReadNode::getLabel() const
    {
        std::stringstream ss;
        ss << _name << ": " << _path.filename().string();
        return ss.str();
    }

    OIIO::ImageBuf ReadNode::exec()
    {
        OIIO::ImageBuf out;

        if (_ffRead)
        {
            OTIO_NS::RationalTime offsetTime = _time;
            if (!_timeOffset.is_invalid_time())
            {
                offsetTime -= _timeOffset;
            }

            out = _ffRead->getImage(offsetTime);

            if (3 == _spec.nchannels)
            {
                // Add an alpha channel.
                const int channelOrder[] = { 0, 1, 2, -1 };
                const float channelValues[] = { 0, 0, 0, 1.0 };
                const std::string channelNames[] = { "", "", "", "A" };
                out = OIIO::ImageBufAlgo::channels(out, 4, channelOrder, channelValues, channelNames);
            }
        }
        else
        {
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

            if (3 == _spec.nchannels)
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

    SequenceReadNode::SequenceReadNode(
        const std::string& base,
        const std::string& namePrefix,
        const std::string& nameSuffix,
        int startFrame,
        int frameStep,
        double rate,
        int frameZeroPadding,
        const MemoryReferences& memoryReferences,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode("SequenceRead", inputs),
        _base(base),
        _namePrefix(namePrefix),
        _nameSuffix(nameSuffix),
        _startFrame(startFrame),
        _frameStep(frameStep),
        _rate(rate),
        _frameZeroPadding(frameZeroPadding),
        _memoryReferences(memoryReferences)
    {
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

    const OIIO::ImageSpec& SequenceReadNode::getSpec() const
    {
        return _spec;
    }

    OIIO::ImageBuf SequenceReadNode::exec()
    {
        OIIO::ImageBuf out;

        OTIO_NS::RationalTime offsetTime = _time;
        if (!_timeOffset.is_invalid_time())
        {
            offsetTime -= _timeOffset;
        }

        const int frame = offsetTime.floor().to_frames();
        const std::string url = getSequenceFrame(
            _base,
            _namePrefix,
            frame,
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
}
