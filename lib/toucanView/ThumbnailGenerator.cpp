// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ThumbnailGenerator.h"

#include <toucanRender/Read.h>

#include <OpenImageIO/imagebufalgo.h>

#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

#include <sstream>

namespace toucan
{
    std::string getThumbnailCacheKey(
        const OTIO_NS::MediaReference* ref,
        const OTIO_NS::RationalTime& time,
        int height)
    {
        std::vector<std::string> s;
        std::stringstream ss;
        ss << ref;
        s.push_back(ss.str());
        s.push_back(ftk::Format("{0}@{1}").arg(time.value()).arg(time.rate()));
        s.push_back(ftk::Format("{0}").arg(height));
        return ftk::join(s, '_');
    }

    ThumbnailGenerator::ThumbnailGenerator(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<TimelineWrapper>& timelineWrapper) :
        _timelineWrapper(timelineWrapper)
    {
        _logSystem = context->getSystem<ftk::LogSystem>();

        _thread.running = true;
        _thread.readCache.setMax(100);
        _thread.thread = std::thread(
            [this]
            {
                while (_thread.running)
                {
                    _run();
                }
                {
                    std::unique_lock<std::mutex> lock(_mutex.mutex);
                    _mutex.stopped = true;
                }
                _cancel();
            });
    }

    ThumbnailGenerator::~ThumbnailGenerator()
    {
        _thread.running = false;
        if (_thread.thread.joinable())
        {
            _thread.thread.join();
        }
    }

    std::future<float> ThumbnailGenerator::getAspect(
        const OTIO_NS::MediaReference* ref)
    {
        auto request = std::make_shared<AspectRequest>();
        request->ref = ref;
        auto out = request->promise.get_future();
        bool valid = false;
        {
            std::unique_lock<std::mutex> lock(_mutex.mutex);
            if (!_mutex.stopped)
            {
                valid = true;
                _mutex.aspectRequests.push_back(request);
            }
        }
        if (valid)
        {
            _thread.cv.notify_one();
        }
        else
        {
            request->promise.set_value(0.F);
        }
        return out;
    }

    ThumbnailRequest ThumbnailGenerator::getThumbnail(
        const OTIO_NS::MediaReference* ref,
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::TimeRange& availableRange,
        int height)
    {
        _requestId++;
        auto request = std::make_shared<Request>();
        request->id = _requestId;
        request->ref = ref;
        request->time = time;
        request->availableRange = availableRange;
        request->height = height;
        ThumbnailRequest out;
        out.id = _requestId;
        out.height = height;
        out.time = time;
        out.future = request->promise.get_future();
        bool valid = false;
        {
            std::unique_lock<std::mutex> lock(_mutex.mutex);
            if (!_mutex.stopped)
            {
                valid = true;
                _mutex.requests.push_back(request);
            }
        }
        if (valid)
        {
            _thread.cv.notify_one();
        }
        else
        {
            request->promise.set_value(nullptr);
        }
        return out;
    }

    void ThumbnailGenerator::cancelThumbnails(const std::vector<uint64_t>& ids)
    {
        if (!ids.empty())
        {
            std::unique_lock<std::mutex> lock(_mutex.mutex);
            auto i = _mutex.requests.begin();
            while (i != _mutex.requests.end())
            {
                const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                if (j != ids.end())
                {
                    i = _mutex.requests.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }

    void ThumbnailGenerator::_run()
    {
        std::shared_ptr<AspectRequest> aspectRequest;
        std::shared_ptr<Request> request;
        {
            std::unique_lock<std::mutex> lock(_mutex.mutex);
            if (_thread.cv.wait_for(
                lock,
                std::chrono::milliseconds(5),
                [this]
                {
                    return
                        !_mutex.aspectRequests.empty() ||
                        !_mutex.requests.empty();
                }))
            {
                if (!_mutex.aspectRequests.empty())
                {
                    aspectRequest = _mutex.aspectRequests.front();
                    _mutex.aspectRequests.pop_front();
                }
                if (!_mutex.requests.empty())
                {
                    request = _mutex.requests.front();
                    _mutex.requests.pop_front();
                }
            }
        }
        if (aspectRequest)
        {
            std::shared_ptr<IReadNode> read;
            try
            {
                read = _timelineWrapper->createReadNode(aspectRequest->ref);
            }
            catch (const std::exception& e)
            {
                _logSystem->print(
                    "toucan::ThumbnailGenerator",
                    e.what(),
                    ftk::LogType::Error);
            }
            float aspect = 0.F;
            if (read)
            {
                read->setTime(read->getTimeRange().start_time());
                const OIIO::ImageBuf buf = read->exec();
                const OIIO::ImageSpec& spec = buf.spec();
                if (spec.height > 0)
                {
                    aspect = spec.width / static_cast<float>(spec.height);
                }
            }
            aspectRequest->promise.set_value(aspect);
        }
        if (request)
        {
            std::shared_ptr<IReadNode> read;
            if (!_thread.readCache.get(request->ref, read))
            {
                try
                {
                    read = _timelineWrapper->createReadNode(request->ref);
                    _thread.readCache.add(request->ref, read);
                }
                catch (const std::exception& e)
                {
                    _logSystem->print(
                        "toucan::ThumbnailGenerator",
                        e.what(),
                        ftk::LogType::Error);
                }
            }

            OIIO::ImageBuf buf;
            if (read)
            {
                //! \bug Workaround for files that are missing timecode.
                OTIO_NS::RationalTime t = request->time;
                if (request->availableRange.start_time() !=
                    read->getTimeRange().start_time())
                {
                    t -= request->availableRange.start_time();
                }

                read->setTime(t);
                buf = read->exec();
            }

            std::shared_ptr<ftk::Image> thumbnail;
            const auto& spec = buf.spec();
            if (spec.width > 0 && spec.height > 0)
            {
                const float aspect = spec.width / static_cast<float>(spec.height);
                const ftk::Size2I thumbnailSize(request->height * aspect, request->height);
                ftk::ImageInfo info;
                info.size = thumbnailSize;
                switch (spec.nchannels)
                {
                case 1:
                    switch (spec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = ftk::ImageType::L_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = ftk::ImageType::L_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = ftk::ImageType::L_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = ftk::ImageType::L_F32; break;
                    }
                    break;
                case 2:
                    switch (spec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = ftk::ImageType::LA_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = ftk::ImageType::LA_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = ftk::ImageType::LA_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = ftk::ImageType::LA_F32; break;
                    }
                    break;
                case 3:
                    switch (spec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = ftk::ImageType::RGB_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = ftk::ImageType::RGB_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = ftk::ImageType::RGB_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = ftk::ImageType::RGB_F32; break;
                    }
                    break;
                default:
                    switch (spec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = ftk::ImageType::RGBA_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = ftk::ImageType::RGBA_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = ftk::ImageType::RGBA_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = ftk::ImageType::RGBA_F32; break;
                    }
                    break;
                }
                info.layout.mirror.y = true;

                if (info.isValid())
                {
                    thumbnail = ftk::Image::create(info);
                    auto resizedBuf = OIIO::ImageBufAlgo::resize(
                        buf,
                        "",
                        0.F,
                        OIIO::ROI(
                            0, info.size.w,
                            0, info.size.h,
                            0, 1,
                            0, std::min(4, spec.nchannels)));
                    memcpy(
                        thumbnail->getData(),
                        resizedBuf.localpixels(),
                        thumbnail->getByteCount());
                }
            }
            request->promise.set_value(thumbnail);
        }
    }

    void ThumbnailGenerator::_cancel()
    {
        std::list<std::shared_ptr<Request> > requests;
        {
            std::unique_lock<std::mutex> lock(_mutex.mutex);
            requests = std::move(_mutex.requests);
        }
        for (auto& request : requests)
        {
            request->promise.set_value(nullptr);
        }
    }
}
