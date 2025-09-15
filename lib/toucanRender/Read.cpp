// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Read.h"

#include "TimelineWrapper.h"
#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

#include <sstream>

namespace toucan
{
    IReadNode::IReadNode(const std::string& name) :
        IImageNode(name)
    {}

    IReadNode::~IReadNode()
    {}

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
        const MemoryReference& memoryReference) :
        IReadNode("ImageRead"),
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
        return { ".exr", ".tif", ".tiff", ".jpg", ".jpeg", ".png" };
    }

    SequenceReadNode::SequenceReadNode(
        const std::string& base,
        const std::string& namePrefix,
        const std::string& nameSuffix,
        int startFrame,
        int frameStep,
        double rate,
        int frameZeroPadding,
        const MemoryReferences& memoryReferences) :
        IReadNode("SequenceRead"),
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

        // Open the sequence file.
        const std::string url = getSequenceFrame(
            _base,
            _namePrefix,
            _time.to_frames(),
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
        return { ".exr", ".tif", ".tiff", ".jpg", ".jpeg", ".png" };
    }

    SVGReadNode::SVGReadNode(
        const std::filesystem::path& path,
        const MemoryReference& memoryReference) :
        IReadNode("SVGRead"),
        _path(path)
    {
        if (memoryReference.isValid())
        {
            _svg = lunasvg::Document::loadFromData(
                reinterpret_cast<const char*>(memoryReference.getData()),
                memoryReference.getSize());
        }
        else
        {
            _svg = lunasvg::Document::loadFromFile(path.u8string());
        }
        if (!_svg)
        {
            std::stringstream ss;
            ss << "Cannot open file: " << _path.string();
            throw std::runtime_error(ss.str());
        }
        _spec = OIIO::ImageSpec(
            _svg->width(),
            _svg->height(),
            4,
            OIIO::TypeDesc::BASETYPE::UINT8);
    }

    SVGReadNode::~SVGReadNode()
    {}

    std::string SVGReadNode::getLabel() const
    {
        std::stringstream ss;
        ss << "Read: " << _path.filename().string();
        return ss.str();
    }

    OIIO::ImageBuf SVGReadNode::exec()
    {
        OIIO::ImageBuf out;
        
        const int w = _svg->width();
        const int h = _svg->height();
        auto bitmap = _svg->renderToBitmap(w, h, 0x00000000);
        if (!bitmap.isNull())
        {
            out = OIIO::ImageBuf(_spec);
            for (int y = 0; y < h; ++y)
            {
                uint8_t* imageP = reinterpret_cast<uint8_t*>(out.localpixels()) + y * w * 4;
                const uint8_t* bitmapP = bitmap.data() + y * w * 4;
                for (int x = 0; x < w; ++x, imageP += 4, bitmapP += 4)
                {
                    imageP[0] = bitmapP[2];
                    imageP[1] = bitmapP[1];
                    imageP[2] = bitmapP[0];
                    imageP[3] = bitmapP[3];
                }
            }
        }

        return out;
    }

    std::vector<std::string> SVGReadNode::getExtensions()
    {
        return { ".svg" };
    }

    MovieReadNode::MovieReadNode(
        const std::filesystem::path& path,
        const MemoryReference& memoryReference) :
        IReadNode("MovieReadNode"),
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

        // Read the image.
        out = _ffRead->getImage(_time);

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
        return { ".mov", ".mp4", ".m4v", ".y4m" };
    }

    std::shared_ptr<IReadNode> createReadNode(
        const std::filesystem::path& path,
        const MemoryReference& mem)
    {
        std::shared_ptr<IReadNode> out;
        if (hasExtension(path.extension().string(), MovieReadNode::getExtensions()))
        {
            out = std::make_shared<MovieReadNode>(path, mem);
        }
        else if (hasExtension(path.extension().string(), ImageReadNode::getExtensions()))
        {
            out = std::make_shared<ImageReadNode>(path, mem);
        }
        else if (hasExtension(path.extension().string(), SVGReadNode::getExtensions()))
        {
            out = std::make_shared<SVGReadNode>(path, mem);
        }
        return out;
    }

    std::shared_ptr<IReadNode> createReadNode(
        const std::string& base,
        const std::string& namePrefix,
        const std::string& nameSuffix,
        int startFrame,
        int frameStep,
        double rate,
        int frameZeroPadding,
        const MemoryReferences& mem)
    {
        std::shared_ptr<IReadNode> out;
        if (hasExtension(nameSuffix, SequenceReadNode::getExtensions()))
        {
            out = std::make_shared<SequenceReadNode>(
                base,
                namePrefix,
                nameSuffix,
                startFrame,
                frameStep,
                rate,
                frameZeroPadding,
                mem);
        }
        return out;

    }

    bool hasExtension(
        const std::string& extension,
        const std::vector<std::string>& extensions)
    {
        const auto i = std::find(extensions.begin(), extensions.end(), toLower(extension));
        return i != extensions.end();
    }
}
