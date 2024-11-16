// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "MemoryMap.h"

#include <opentimelineio/anyVector.h>

#include <OpenImageIO/imagebuf.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

} // extern "C"

#include <filesystem>

namespace toucan
{
    class FFmpegRead : public std::enable_shared_from_this<FFmpegRead>
    {
    public:
        FFmpegRead(
            const std::filesystem::path&,
            const MemoryReference& = {});

        virtual ~FFmpegRead();

        const OIIO::ImageSpec& getSpec();
        const OTIO_NS::TimeRange& getTimeRange() const;

        OIIO::ImageBuf getImage(const OTIO_NS::RationalTime&);

    private:
        void _seek(const OTIO_NS::RationalTime&);
        OIIO::ImageBuf _read();

        std::filesystem::path _path;
        MemoryReference _memoryReference;
        OIIO::ImageSpec _spec;
        OTIO_NS::TimeRange _timeRange;
        OTIO_NS::RationalTime _currentTime;

        struct AVIOBufferData
        {
            AVIOBufferData();
            AVIOBufferData(const uint8_t*, size_t size);

            const uint8_t* data = nullptr;
            size_t size = 0;
            size_t offset = 0;
        };
        static int _avIOBufferRead(void* opaque, uint8_t* buf, int bufSize);
        static int64_t _avIOBufferSeek(void* opaque, int64_t offset, int whence);

        AVFormatContext* _avFormatContext = nullptr;
        AVIOBufferData _avIOBufferData;
        uint8_t* _avIOContextBuffer = nullptr;
        AVIOContext* _avIOContext = nullptr;
        AVRational _avSpeed = { 24, 1 };
        int _avStream = -1;
        std::map<int, AVCodecParameters*> _avCodecParameters;
        std::map<int, AVCodecContext*> _avCodecContext;
        AVFrame* _avFrame = nullptr;
        AVFrame* _avFrame2 = nullptr;
        AVPixelFormat _avInputPixelFormat = AV_PIX_FMT_NONE;
        AVPixelFormat _avOutputPixelFormat = AV_PIX_FMT_NONE;
        SwsContext* _swsContext = nullptr;
        bool _eof = false;
    };
}
