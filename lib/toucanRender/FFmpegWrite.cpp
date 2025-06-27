// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FFmpegWrite.h"

#include <OpenImageIO/imagebufalgo.h>

#include <feather-tk/core/Time.h>

#include <iostream>
#include <sstream>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace toucan
{
    namespace ffmpeg
    {
        Write::Write(
            const std::filesystem::path& path,
            const OIIO::ImageSpec& spec,
            const OTIO_NS::TimeRange& timeRange,
            VideoCodec videoCodec) :
            _path(path),
            _spec(spec),
            _timeRange(timeRange)
        {
            av_log_set_level(AV_LOG_QUIET);
            //av_log_set_level(AV_LOG_VERBOSE);
            //av_log_set_callback(log);

            AVCodecID avCodecID = getVideoCodecId(videoCodec);
            int avProfile = getVideoCodecProfile(videoCodec);

            int r = avformat_alloc_output_context2(&_avFormatContext, NULL, NULL, _path.string().c_str());
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }
            const AVCodec* avCodec = avcodec_find_encoder(avCodecID);
            if (!avCodec)
            {
                throw std::runtime_error("Cannot find encoder");
            }
            _avCodecContext = avcodec_alloc_context3(avCodec);
            if (!_avCodecContext)
            {
                throw std::runtime_error("Cannot allocate context");
            }
            _avVideoStream = avformat_new_stream(_avFormatContext, avCodec);
            if (!_avVideoStream)
            {
                throw std::runtime_error("Cannot allocate stream");
            }
            if (!avCodec->pix_fmts)
            {
                throw std::runtime_error("No pixel formats available");
            }

            _avCodecContext->codec_id = avCodec->id;
            _avCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            _avCodecContext->width = spec.width;
            _avCodecContext->height = spec.height;
            _avCodecContext->sample_aspect_ratio = AVRational({ 1, 1 });
            _avCodecContext->pix_fmt = avCodec->pix_fmts[0];
            const auto rational = feather_tk::toRational(timeRange.duration().rate());
            _avCodecContext->time_base = { rational.second, rational.first };
            _avCodecContext->framerate = { rational.first, rational.second };
            _avCodecContext->profile = avProfile;
            if (_avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            {
                _avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            _avCodecContext->thread_count = 0;
            _avCodecContext->thread_type = FF_THREAD_FRAME;

            r = avcodec_open2(_avCodecContext, avCodec, NULL);
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }

            r = avcodec_parameters_from_context(_avVideoStream->codecpar, _avCodecContext);
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }

            _avVideoStream->time_base = { rational.second, rational.first };
            _avVideoStream->avg_frame_rate = { rational.first, rational.second };

            //av_dump_format(_avFormatContext, 0, _path.string().c_str(), 1);

            r = avio_open(&_avFormatContext->pb, _path.string().c_str(), AVIO_FLAG_WRITE);
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }

            r = avformat_write_header(_avFormatContext, NULL);
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }

            _avPacket = av_packet_alloc();
            if (!_avPacket)
            {
                throw std::runtime_error("Cannot allocate packet");
            }

            _avFrame = av_frame_alloc();
            if (!_avFrame)
            {
                throw std::runtime_error("Cannot allocate frame");
            }
            _avFrame->format = _avVideoStream->codecpar->format;
            _avFrame->width = _avVideoStream->codecpar->width;
            _avFrame->height = _avVideoStream->codecpar->height;
            r = av_frame_get_buffer(_avFrame, 0);
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }

            _avFrame2 = av_frame_alloc();
            if (!_avFrame2)
            {
                throw std::runtime_error("Cannot allocate frame");
            }

            _opened = true;
        }

        Write::~Write()
        {
            if (_opened)
            {
                _encodeVideo(nullptr);
                av_write_trailer(_avFormatContext);
            }
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
            if (_avPacket)
            {
                av_packet_free(&_avPacket);
            }
            if (_avCodecContext)
            {
                avcodec_free_context(&_avCodecContext);
            }
            if (_avFormatContext && _avFormatContext->pb)
            {
                avio_closep(&_avFormatContext->pb);
            }
            if (_avFormatContext)
            {
                avformat_free_context(_avFormatContext);
            }
        }

        void Write::writeImage(const OIIO::ImageBuf& buf, const OTIO_NS::RationalTime& time)
        {
            auto spec = buf.spec();
            const OIIO::ImageBuf* bufP = &buf;
            OIIO::ImageBuf tmp;
            switch (spec.format.basetype)
            {
            case OIIO::TypeDesc::INT8:
                spec.format.basetype = OIIO::TypeDesc::UINT8;
                break;
            case OIIO::TypeDesc::INT16:
            case OIIO::TypeDesc::UINT32:
            case OIIO::TypeDesc::INT32:
            case OIIO::TypeDesc::UINT64:
            case OIIO::TypeDesc::INT64:
            case OIIO::TypeDesc::HALF:
            case OIIO::TypeDesc::FLOAT:
                spec.format.basetype = OIIO::TypeDesc::UINT16;
                break;
            }
            if (buf.spec().format.basetype != spec.format.basetype)
            {
                tmp = OIIO::ImageBufAlgo::copy(buf, spec.format);
                bufP = &tmp;
            }

            AVPixelFormat avPixelFormatIn = AV_PIX_FMT_NONE;
            switch (spec.nchannels)
            {
            case 1:
                switch (spec.format.basetype)
                {
                case OIIO::TypeDesc::UINT8: avPixelFormatIn = AV_PIX_FMT_GRAY8; break;
                case OIIO::TypeDesc::UINT16: avPixelFormatIn = AV_PIX_FMT_GRAY16; break;
                default: break;
                }
                break;
            case 3:
                switch (spec.format.basetype)
                {
                case OIIO::TypeDesc::UINT8: avPixelFormatIn = AV_PIX_FMT_RGB24; break;
                case OIIO::TypeDesc::UINT16: avPixelFormatIn = AV_PIX_FMT_RGB48; break;
                default: break;
                }
                break;
            case 4:
                switch (spec.format.basetype)
                {
                case OIIO::TypeDesc::UINT8: avPixelFormatIn = AV_PIX_FMT_RGBA; break;
                case OIIO::TypeDesc::UINT16: avPixelFormatIn = AV_PIX_FMT_RGBA64; break;
                default: break;
                }
                break;
            default: break;
            }
            if (AV_PIX_FMT_NONE == avPixelFormatIn)
            {
                throw std::runtime_error("Incompatible pixel type");
            }
            if (spec.width != _spec.width ||
                spec.height != _spec.height ||
                avPixelFormatIn != _avPixelFormatIn)
            {
                _avPixelFormatIn = avPixelFormatIn;
                if (_swsContext)
                {
                    sws_freeContext(_swsContext);
                }
                /*_swsContext = sws_getContext(
                    spec.width,
                    spec.height,
                    _avPixelFormatIn,
                    spec.width,
                    spec.height,
                    _avCodecContext->pix_fmt,
                    swsScaleFlags,
                    0,
                    0,
                    0);*/
                _swsContext = sws_alloc_context();
                if (!_swsContext)
                {
                    throw std::runtime_error("Cannot allocate context");
                }
                av_opt_set_defaults(_swsContext);
                int r = av_opt_set_int(_swsContext, "srcw", spec.width, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "srch", spec.height, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "src_format", _avPixelFormatIn, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "dstw", spec.width, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "dsth", spec.height, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "dst_format", _avCodecContext->pix_fmt, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "sws_flags", SWS_BICUBIC, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(_swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
                r = sws_init_context(_swsContext, nullptr, nullptr);
                if (r < 0)
                {
                    throw std::runtime_error(getErrorLabel(r));
                }
            }

            av_image_fill_arrays(
                _avFrame2->data,
                _avFrame2->linesize,
                reinterpret_cast<const uint8_t*>(bufP->localpixels()),
                _avPixelFormatIn,
                spec.width,
                spec.height,
                1);

            sws_scale(
                _swsContext,
                (uint8_t const* const*)_avFrame2->data,
                _avFrame2->linesize,
                0,
                _spec.height,
                _avFrame->data,
                _avFrame->linesize);

            const auto timeRational = feather_tk::toRational(time.rate());
            _avFrame->pts = av_rescale_q(
                (time - _timeRange.start_time()).value(),
                { timeRational.second, timeRational.first },
                _avVideoStream->time_base);
            _encodeVideo(_avFrame);
        }

        void Write::_encodeVideo(AVFrame* frame)
        {
            int r = avcodec_send_frame(_avCodecContext, frame);
            if (r < 0)
            {
                throw std::runtime_error(getErrorLabel(r));
            }

            while (r >= 0)
            {
                r = avcodec_receive_packet(_avCodecContext, _avPacket);
                if (r == AVERROR(EAGAIN) || r == AVERROR_EOF)
                {
                    return;
                }
                else if (r < 0)
                {
                    throw std::runtime_error(getErrorLabel(r));
                }
                r = av_interleaved_write_frame(_avFormatContext, _avPacket);
                if (r < 0)
                {
                    throw std::runtime_error(getErrorLabel(r));
                }
                av_packet_unref(_avPacket);
            }
        }
    }
}
