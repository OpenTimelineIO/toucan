// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>
#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>

#include <feather-tk/core/Image.h>
#include <feather-tk/core/LogSystem.h>

#include <atomic>
#include <filesystem>
#include <future>
#include <list>
#include <mutex>
#include <thread>

namespace toucan
{
    //! Get a thumbnail cache key.
    std::string getThumbnailCacheKey(
        const OTIO_NS::MediaReference*,
        const OTIO_NS::RationalTime&,
        int height);

    //! Thumbnail request.
    struct ThumbnailRequest
    {
        uint64_t id = 0;
        OTIO_NS::RationalTime time;
        int height = 0;
        std::future<std::shared_ptr<ftk::Image> > future;
    };

    //! Thumbnail generator.
    class ThumbnailGenerator : public std::enable_shared_from_this<ThumbnailGenerator>
    {
    public:
        ThumbnailGenerator(
            const std::shared_ptr<ftk::Context>&,
            const std::filesystem::path&,
            const std::shared_ptr<TimelineWrapper>&,
            const std::shared_ptr<ImageEffectHost>&);

        ~ThumbnailGenerator();

        //! Get the timeline aspect ratio.
        float getAspect() const;

        //! Get a timeline thumbnail.
        ThumbnailRequest getThumbnail(
            const OTIO_NS::RationalTime&,
            int height);

        //! Get a media aspect ratio.
        std::future<float> getAspect(
            const OTIO_NS::MediaReference*,
            const OTIO_NS::RationalTime&);

        //! Get a media thumbnail.
        ThumbnailRequest getThumbnail(
            const OTIO_NS::MediaReference*,
            const OTIO_NS::RationalTime&,
            int height);

        //! Cancel thumbnail requests.
        void cancelThumbnails(const std::vector<uint64_t>&);

    private:
        void _run();
        void _cancel();

        std::shared_ptr<IImageNode> _findNode(
            const std::shared_ptr<IImageNode>&,
            const OTIO_NS::MediaReference*);

        std::shared_ptr<ftk::LogSystem> _logSystem;
        std::filesystem::path _path;
        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        std::shared_ptr<ImageEffectHost> _host;
        std::shared_ptr<ImageGraph> _graph;
        float _aspect = 1.F;

        struct AspectRequest
        {
            const OTIO_NS::MediaReference* ref = nullptr;
            OTIO_NS::RationalTime time;
            std::promise<float> promise;
        };

        struct Request
        {
            uint64_t id = 0;
            const OTIO_NS::MediaReference* ref = nullptr;
            OTIO_NS::RationalTime time;
            int height = 0;
            std::promise<std::shared_ptr<ftk::Image> > promise;
        };
        uint64_t _requestId = 0;

        struct Mutex
        {
            std::list<std::shared_ptr<AspectRequest> > aspectRequests;
            std::list<std::shared_ptr<Request> > requests;
            bool stopped = false;
            std::mutex mutex;
        };
        Mutex _mutex;

        struct Thread
        {
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        Thread _thread;
    };
}
