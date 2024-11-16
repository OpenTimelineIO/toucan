// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FFmpegRead.h"

#include "Util.h"

#include <iostream>
#include <sstream>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace toucan
{
    namespace
    {
        const size_t avIOContextBufferSize = 4096;

        void logCallback(void*, int level, const char* fmt, va_list vl)
        {
            switch (level)
            {
            case AV_LOG_PANIC:
            case AV_LOG_FATAL:
            case AV_LOG_ERROR:
            case AV_LOG_WARNING:
            case AV_LOG_INFO:
                {
                    char buf[4096];
                    vsnprintf(buf, 4096, fmt, vl);
                    std::cout << buf;
                }
                break;
            case AV_LOG_VERBOSE:
            default: break;
            }
        }

        AVRational swap(AVRational value)
        {
            return AVRational({ value.den, value.num });
        }

        class Packet
        {
        public:
            Packet()
            {
                p = av_packet_alloc();
            }

            ~Packet()
            {
                av_packet_free(&p);
            }

            AVPacket* p = nullptr;
        };

        size_t getByteCount(const OIIO::ImageSpec& spec)
        {
            size_t type = 0;
            switch (spec.format.basetype)
            {

            }
            return spec.width * spec.height * type;
        }
    }

    FFmpegRead::FFmpegRead(
        const std::filesystem::path& path,
        const MemoryReference& memoryReference) :
        _path(path),
        _memoryReference(memoryReference)
    {
        av_log_set_level(AV_LOG_QUIET);
        //av_log_set_level(AV_LOG_VERBOSE);
        //av_log_set_callback(logCallback);

        if (memoryReference.isValid())
        {
            _avFormatContext = avformat_alloc_context();
            if (!_avFormatContext)
            {
                throw std::runtime_error("Cannot allocate format context");
            }

            _avIOBufferData = AVIOBufferData(
                reinterpret_cast<const uint8_t*>(memoryReference.getData()),
                memoryReference.getSize());
            _avIOContextBuffer = static_cast<uint8_t*>(av_malloc(avIOContextBufferSize));
            _avIOContext = avio_alloc_context(
                _avIOContextBuffer,
                avIOContextBufferSize,
                0,
                &_avIOBufferData,
                &_avIOBufferRead,
                nullptr,
                &_avIOBufferSeek);
            if (!_avIOContext)
            {
                throw std::runtime_error("Cannot allocate I/O context");
            }

            _avFormatContext->pb = _avIOContext;
        }

        const std::string fileName = path.string();
        int r = avformat_open_input(
            &_avFormatContext,
            !_avFormatContext ? fileName.c_str() : nullptr,
            nullptr,
            nullptr);
        if (r < 0)
        {
            throw std::runtime_error("Cannot open file");
        }

        r = avformat_find_stream_info(_avFormatContext, nullptr);
        if (r < 0)
        {
            throw std::runtime_error("Cannot find stream info");
        }
        for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
        {
            //av_dump_format(_avFormatContext, 0, fileName.c_str(), 0);
            if (AVMEDIA_TYPE_VIDEO == _avFormatContext->streams[i]->codecpar->codec_type &&
                AV_DISPOSITION_DEFAULT == _avFormatContext->streams[i]->disposition)
            {
                _avStream = i;
                break;
            }
        }
        if (-1 == _avStream)
        {
            for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_VIDEO == _avFormatContext->streams[i]->codecpar->codec_type)
                {
                    _avStream = i;
                    break;
                }
            }
        }

        int dataStream = -1;
        for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
        {
            if (AVMEDIA_TYPE_DATA == _avFormatContext->streams[i]->codecpar->codec_type &&
                AV_DISPOSITION_DEFAULT == _avFormatContext->streams[i]->disposition)
            {
                dataStream = i;
                break;
            }
        }
        if (-1 == dataStream)
        {
            for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_DATA == _avFormatContext->streams[i]->codecpar->codec_type)
                {
                    dataStream = i;
                    break;
                }
            }
        }
        std::string timecode;
        if (dataStream != -1)
        {
            AVDictionaryEntry* tag = nullptr;
            while ((tag = av_dict_get(
                _avFormatContext->streams[dataStream]->metadata,
                "",
                tag,
                AV_DICT_IGNORE_SUFFIX)))
            {
                if ("timecode" == toLower(tag->key))
                {
                    timecode = tag->value;
                    break;
                }
            }
        }

        if (_avStream != -1)
        {
            //av_dump_format(_avFormatContext, _avStream, fileName.c_str(), 0);

            auto avVideoStream = _avFormatContext->streams[_avStream];
            auto avVideoCodecParameters = avVideoStream->codecpar;
            auto avVideoCodec = avcodec_find_decoder(avVideoCodecParameters->codec_id);
            if (!avVideoCodec)
            {
                throw std::runtime_error("No video codec found");
            }
            _avCodecParameters[_avStream] = avcodec_parameters_alloc();
            if (!_avCodecParameters[_avStream])
            {
                throw std::runtime_error("Cannot allocate parameters");
            }
            avcodec_parameters_copy(_avCodecParameters[_avStream], avVideoCodecParameters);
            _avCodecContext[_avStream] = avcodec_alloc_context3(avVideoCodec);
            if (!_avCodecParameters[_avStream])
            {
                throw std::runtime_error("Cannot allocate context");
            }
            avcodec_parameters_to_context(_avCodecContext[_avStream], _avCodecParameters[_avStream]);
            _avCodecContext[_avStream]->thread_count = 0;
            _avCodecContext[_avStream]->thread_type = FF_THREAD_FRAME;
            r = avcodec_open2(_avCodecContext[_avStream], avVideoCodec, 0);
            if (r < 0)
            {
                throw std::runtime_error("Cannot open stream");
            }

            int width = _avCodecParameters[_avStream]->width;
            int height = _avCodecParameters[_avStream]->height;
            double pixelAspectRatio = 1.0;
            if (_avCodecParameters[_avStream]->sample_aspect_ratio.den > 0 &&
                _avCodecParameters[_avStream]->sample_aspect_ratio.num > 0)
            {
                pixelAspectRatio = av_q2d(_avCodecParameters[_avStream]->sample_aspect_ratio);
            }

            _avInputPixelFormat = static_cast<AVPixelFormat>(_avCodecParameters[_avStream]->format);
            int nchannels = 0;
            OIIO::TypeDesc format = OIIO::TypeDesc::UNKNOWN;
            switch (_avInputPixelFormat)
            {
            case AV_PIX_FMT_RGB24:
                _avOutputPixelFormat = _avInputPixelFormat;
                nchannels = 3;
                format = OIIO::TypeUInt8;
                break;
            case AV_PIX_FMT_GRAY8:
                _avOutputPixelFormat = _avInputPixelFormat;
                nchannels = 1;
                format = OIIO::TypeUInt8;
                break;
            case AV_PIX_FMT_RGBA:
                _avOutputPixelFormat = _avInputPixelFormat;
                nchannels = 4;
                format = OIIO::TypeUInt8;
                break;
            case AV_PIX_FMT_YUV420P:
            case AV_PIX_FMT_YUV422P:
            case AV_PIX_FMT_YUV444P:
                _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                nchannels = 3;
                format = OIIO::TypeUInt8;
                break;
            case AV_PIX_FMT_YUV420P10BE:
            case AV_PIX_FMT_YUV420P10LE:
            case AV_PIX_FMT_YUV420P12BE:
            case AV_PIX_FMT_YUV420P12LE:
            case AV_PIX_FMT_YUV420P16BE:
            case AV_PIX_FMT_YUV420P16LE:
            case AV_PIX_FMT_YUV422P10BE:
            case AV_PIX_FMT_YUV422P10LE:
            case AV_PIX_FMT_YUV422P12BE:
            case AV_PIX_FMT_YUV422P12LE:
            case AV_PIX_FMT_YUV422P16BE:
            case AV_PIX_FMT_YUV422P16LE:
            case AV_PIX_FMT_YUV444P10BE:
            case AV_PIX_FMT_YUV444P10LE:
            case AV_PIX_FMT_YUV444P12BE:
            case AV_PIX_FMT_YUV444P12LE:
            case AV_PIX_FMT_YUV444P16BE:
            case AV_PIX_FMT_YUV444P16LE:
                _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                nchannels = 3;
                format = OIIO::TypeUInt16;
                break;
            case AV_PIX_FMT_YUVA420P:
            case AV_PIX_FMT_YUVA422P:
            case AV_PIX_FMT_YUVA444P:
                _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                nchannels = 4;
                format = OIIO::TypeUInt8;
                break;
            case AV_PIX_FMT_YUVA444P10BE:
            case AV_PIX_FMT_YUVA444P10LE:
            case AV_PIX_FMT_YUVA444P12BE:
            case AV_PIX_FMT_YUVA444P12LE:
            case AV_PIX_FMT_YUVA444P16BE:
            case AV_PIX_FMT_YUVA444P16LE:
                _avOutputPixelFormat = AV_PIX_FMT_RGBA64;
                nchannels = 4;
                format = OIIO::TypeUInt16;
                break;
            default:
                _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                nchannels = 3;
                format = OIIO::TypeUInt8;
                break;
            }
            _spec = OIIO::ImageSpec(width, height, nchannels, format);

            _avSpeed = av_guess_frame_rate(_avFormatContext, avVideoStream, nullptr);
            const double speed = av_q2d(_avSpeed);

            std::size_t frameCount = 0;
            if (avVideoStream->nb_frames > 0)
            {
                frameCount = avVideoStream->nb_frames;
            }
            else if (avVideoStream->duration != AV_NOPTS_VALUE)
            {
                frameCount = av_rescale_q(
                    avVideoStream->duration,
                    avVideoStream->time_base,
                    swap(avVideoStream->r_frame_rate));
            }
            else if (_avFormatContext->duration != AV_NOPTS_VALUE)
            {
                frameCount = av_rescale_q(
                    _avFormatContext->duration,
                    av_get_time_base_q(),
                    swap(avVideoStream->r_frame_rate));
            }

            AVDictionaryEntry* tag = nullptr;
            while ((tag = av_dict_get(_avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
            {
                const std::string key(tag->key);
                const std::string value(tag->value);
                if ("timecode" == toLower(key))
                {
                    timecode = value;
                }
            }

            OTIO_NS::RationalTime startTime(0.0, speed);
            if (!timecode.empty())
            {
                opentime::ErrorStatus errorStatus;
                const OTIO_NS::RationalTime time = OTIO_NS::RationalTime::from_timecode(
                    timecode,
                    speed,
                    &errorStatus);
                if (!opentime::is_error(errorStatus))
                {
                    startTime = time.floor();
                }
            }
            _timeRange = OTIO_NS::TimeRange(
                startTime,
                OTIO_NS::RationalTime(frameCount, speed));
            _currentTime = startTime;

            _avFrame = av_frame_alloc();
            if (!_avFrame)
            {
                throw std::runtime_error("Cannot allocate frame");
            }
            _avFrame2 = av_frame_alloc();
            if (!_avFrame2)
            {
                throw std::runtime_error("Cannot allocate frame");
            }
            //! \bug These fields need to be filled out for
            //! sws_scale_frame()?
            _avFrame2->format = _avOutputPixelFormat;
            _avFrame2->width = width;
            _avFrame2->height = height;
            _avFrame2->buf[0] = av_buffer_alloc(_spec.image_bytes());

            _swsContext = sws_alloc_context();
            if (!_swsContext)
            {
                throw std::runtime_error("Cannot allocate context");
            }
            av_opt_set_defaults(_swsContext);
            int r = av_opt_set_int(_swsContext, "srcw", _avCodecParameters[_avStream]->width, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "srch", _avCodecParameters[_avStream]->height, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "src_format", _avInputPixelFormat, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "dstw", _avCodecParameters[_avStream]->width, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "dsth", _avCodecParameters[_avStream]->height, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "dst_format", _avOutputPixelFormat, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "sws_flags", SWS_FAST_BILINEAR, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
            r = sws_init_context(_swsContext, nullptr, nullptr);
            if (r < 0)
            {
                throw std::runtime_error("Cannot initialize sws context");
            }

            const int* inTable = nullptr;
            int        inFull = 0;
            const int* outTable = nullptr;
            int        outFull = 0;
            int        brightness = 0;
            int        contrast = 0;
            int        saturation = 0;
            r = sws_getColorspaceDetails(
                _swsContext,
                (int**)&inTable,
                &inFull,
                (int**)&outTable,
                &outFull,
                &brightness,
                &contrast,
                &saturation);

            AVColorSpace colorSpace = _avCodecParameters[_avStream]->color_space;
            if (AVCOL_SPC_UNSPECIFIED == colorSpace)
            {
                colorSpace = AVCOL_SPC_BT709;
            }
            inFull = 1;
            outFull = 1;

            r = sws_setColorspaceDetails(
                _swsContext,
                sws_getCoefficients(colorSpace),
                inFull,
                sws_getCoefficients(AVCOL_SPC_BT709),
                outFull,
                brightness,
                contrast,
                saturation);
        }
    }

    FFmpegRead::~FFmpegRead()
    {
        if (_swsContext)
        {
            sws_freeContext(_swsContext);
        }
        if (_avFrame2)
        {
            av_frame_free(&_avFrame2);
        }
        if (_avFrame)
        {
            av_frame_free(&_avFrame);
        }
        for (auto i : _avCodecContext)
        {
            avcodec_close(i.second);
            avcodec_free_context(&i.second);
        }
        for (auto i : _avCodecParameters)
        {
            avcodec_parameters_free(&i.second);
        }
        if (_avIOContext)
        {
            avio_context_free(&_avIOContext);
        }
        //! \bug Free'd by avio_context_free()?
        //if (_avIOContextBuffer)
        //{
        //    av_free(_avIOContextBuffer);
        //}
        if (_avFormatContext)
        {
            avformat_close_input(&_avFormatContext);
        }
    }

    const OIIO::ImageSpec& FFmpegRead::getSpec()
    {
        return _spec;
    }

    const OTIO_NS::TimeRange& FFmpegRead::getTimeRange() const
    {
        return _timeRange;
    }

    OIIO::ImageBuf FFmpegRead::getImage(const OTIO_NS::RationalTime& time)
    {
        if (time != _currentTime)
        {
            _seek(time);
        }
        return _read();
    }

    void FFmpegRead::_seek(const OTIO_NS::RationalTime& time)
    {
        if (_avStream != -1)
        {
            avcodec_flush_buffers(_avCodecContext[_avStream]);
            if (av_seek_frame(
                _avFormatContext,
                _avStream,
                av_rescale_q(
                    time.value() - _timeRange.start_time().value(),
                    swap(_avSpeed),
                    _avFormatContext->streams[_avStream]->time_base),
                AVSEEK_FLAG_BACKWARD) < 0)
            {
                //! \todo How should this be handled?
            }
            _currentTime = time;
        }
        _eof = false;
    }

    OIIO::ImageBuf FFmpegRead::_read()
    {
        OIIO::ImageBuf out;
        if (_avStream != -1)
        {
            Packet packet;
            int decoding = 0;
            while (0 == decoding)
            {
                if (!_eof)
                {
                    decoding = av_read_frame(_avFormatContext, packet.p);
                    if (AVERROR_EOF == decoding)
                    {
                        _eof = true;
                        decoding = 0;
                    }
                    else if (decoding < 0)
                    {
                        //! \todo How should this be handled?
                        break;
                    }
                }
                if ((_eof && _avStream != -1) || (_avStream == packet.p->stream_index))
                {
                    decoding = avcodec_send_packet(
                        _avCodecContext[_avStream],
                        _eof ? nullptr : packet.p);
                    if (AVERROR_EOF == decoding)
                    {
                        decoding = 0;
                    }
                    else if (decoding < 0)
                    {
                        //! \todo How should this be handled?
                        break;
                    }
                    
                    while (0 == decoding)
                    {
                        decoding = avcodec_receive_frame(_avCodecContext[_avStream], _avFrame);
                        if (decoding < 0)
                        {
                            break;
                        }
                        const int64_t timestamp = _avFrame->pts != AV_NOPTS_VALUE ? _avFrame->pts : _avFrame->pkt_dts;

                        const OTIO_NS::RationalTime frameTime(
                            _timeRange.start_time().value() +
                            av_rescale_q(
                                timestamp,
                                _avFormatContext->streams[_avStream]->time_base,
                                swap(_avFormatContext->streams[_avStream]->r_frame_rate)),
                            _timeRange.duration().rate());

                        if (frameTime >= _currentTime)
                        {
                            out = OIIO::ImageBuf(_spec);

                            av_image_fill_arrays(
                                _avFrame2->data,
                                _avFrame2->linesize,
                                (const uint8_t*)out.localpixels(),
                                _avOutputPixelFormat,
                                _spec.width,
                                _spec.height,
                                1);
                            sws_scale_frame(_swsContext, _avFrame2, _avFrame);

                            _currentTime += OTIO_NS::RationalTime(1.0, _timeRange.duration().rate());

                            decoding = 1;
                            break;
                        }
                    }

                    if (AVERROR(EAGAIN) == decoding)
                    {
                        decoding = 0;
                    }
                    else if (AVERROR_EOF == decoding)
                    {
                        break;
                    }
                    else if (decoding < 0)
                    {
                        //! \todo How should this be handled?
                        break;
                    }
                    else if (1 == decoding)
                    {
                        break;
                    }
                }
                if (packet.p->buf)
                {
                    av_packet_unref(packet.p);
                }
            }
            if (packet.p->buf)
            {
                av_packet_unref(packet.p);
            }
        }
        return out;
    }

    FFmpegRead::AVIOBufferData::AVIOBufferData()
    {}

    FFmpegRead::AVIOBufferData::AVIOBufferData(const uint8_t* data, size_t size) :
        data(data),
        size(size)
    {}

    int FFmpegRead::_avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
    {
        AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

        const int64_t remaining = bufferData->size - bufferData->offset;
        int bufSizeClamped = std::min(std::max(
            static_cast<int64_t>(bufSize),
            static_cast<int64_t>(0)),
            remaining);
        if (!bufSizeClamped)
        {
            return AVERROR_EOF;
        }

        memcpy(buf, bufferData->data + bufferData->offset, bufSizeClamped);
        bufferData->offset += bufSizeClamped;

        return bufSizeClamped;
    }

    int64_t FFmpegRead::_avIOBufferSeek(void* opaque, int64_t offset, int whence)
    {
        AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

        if (whence & AVSEEK_SIZE)
        {
            return bufferData->size;
        }

        bufferData->offset = std::min(std::max(
            offset,
            static_cast<int64_t>(0)),
            static_cast<int64_t>(bufferData->size));

        return offset;
    }
}
