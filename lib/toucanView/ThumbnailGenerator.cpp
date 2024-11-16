// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ThumbnailGenerator.h"

#include <toucan/ImageGraph.h>

#include <OpenImageIO/imagebufalgo.h>

#include <sstream>

namespace toucan
{
    ThumbnailGenerator::ThumbnailGenerator(
        const std::filesystem::path& path,
        const std::shared_ptr<TimelineWrapper>& timelineWrapper,
        const std::shared_ptr<ImageEffectHost>& host) :
        _path(path),
        _timelineWrapper(timelineWrapper),
        _host(host)
    {
        _thread.running = true;
        _thread.thread = std::thread(
            [this]
            {
                _graph = std::make_shared<ImageGraph>(_path, _timelineWrapper);
                const IMATH_NAMESPACE::V2i& imageSize = _graph->getImageSize();
                _aspect = imageSize.y > 0 ?
                    (imageSize.x / static_cast<float>(imageSize.y)) :
                    0.F;

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

    float ThumbnailGenerator::getAspect() const
    {
        return _aspect;
    }

    ThumbnailRequest ThumbnailGenerator::getThumbnail(
        const OTIO_NS::RationalTime& time,
        int height)
    {
        _requestId++;
        auto request = std::make_shared<Request>();
        request->id = _requestId;
        request->height = height;
        request->time = time;
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
        std::shared_ptr<Request> request;
        {
            std::unique_lock<std::mutex> lock(_mutex.mutex);
            if (_thread.cv.wait_for(
                lock,
                std::chrono::milliseconds(5),
                [this]
                {
                    return !_mutex.requests.empty();
                }))
            {
                request = _mutex.requests.front();
                _mutex.requests.pop_front();
            }
        }
        if (request)
        {
            std::shared_ptr<dtk::Image> thumbnail;
            auto i = _thread.cache.find(std::make_pair(request->time, request->height));
            if (i != _thread.cache.end())
            {
                thumbnail = i->second;
            }
            else
            {
                auto graph = _graph->exec(_host, request->time);
                const auto sourceBuf = graph->exec();
                const auto& sourceSpec = sourceBuf.spec();

                const dtk::Size2I thumbnailSize(request->height * _aspect, request->height);
                dtk::ImageInfo info;
                info.size = thumbnailSize;
                switch (sourceSpec.nchannels)
                {
                case 1:
                    switch (sourceSpec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = dtk::ImageType::L_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = dtk::ImageType::L_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = dtk::ImageType::L_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = dtk::ImageType::L_F32; break;
                    }
                    break;
                case 2:
                    switch (sourceSpec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = dtk::ImageType::LA_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = dtk::ImageType::LA_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = dtk::ImageType::LA_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = dtk::ImageType::LA_F32; break;
                    }
                    break;
                case 3:
                    switch (sourceSpec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = dtk::ImageType::RGB_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = dtk::ImageType::RGB_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = dtk::ImageType::RGB_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = dtk::ImageType::RGB_F32; break;
                    }
                    break;
                default:
                    switch (sourceSpec.format.basetype)
                    {
                    case OIIO::TypeDesc::UINT8: info.type = dtk::ImageType::RGBA_U8; break;
                    case OIIO::TypeDesc::UINT16: info.type = dtk::ImageType::RGBA_U16; break;
                    case OIIO::TypeDesc::HALF: info.type = dtk::ImageType::RGBA_F16; break;
                    case OIIO::TypeDesc::FLOAT: info.type = dtk::ImageType::RGBA_F32; break;
                    }
                    break;
                }
                info.layout.mirror.y = true;

                if (info.isValid())
                {
                    thumbnail = dtk::Image::create(info);
                    auto resizedBuf = OIIO::ImageBufAlgo::resize(
                        sourceBuf,
                        "",
                        0.F,
                        OIIO::ROI(
                            0, info.size.w,
                            0, info.size.h,
                            0, 1,
                            0, std::min(4, sourceSpec.nchannels)));
                    memcpy(
                        thumbnail->getData(),
                        resizedBuf.localpixels(),
                        thumbnail->getByteCount());
                }

                _thread.cache[std::make_pair(request->time, request->height)] = thumbnail;
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
