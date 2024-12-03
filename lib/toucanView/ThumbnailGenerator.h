// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucan/ImageEffectHost.h>
#include <toucan/ImageGraph.h>
#include <toucan/TimelineWrapper.h>

#include <dtk/core/Image.h>

#include <atomic>
#include <filesystem>
#include <future>
#include <list>
#include <mutex>
#include <thread>

namespace toucan
{
    //! Thumbnail request.
    struct ThumbnailRequest
    {
        uint64_t id = 0;
        int height = 0;
        OTIO_NS::RationalTime time;
        std::future<std::shared_ptr<dtk::Image> > future;
    };

    //! Thumbnail generator.
    class ThumbnailGenerator : public std::enable_shared_from_this<ThumbnailGenerator>
    {
    public:
        ThumbnailGenerator(
            const std::filesystem::path&,
            const std::shared_ptr<TimelineWrapper>&,
            const std::shared_ptr<ImageEffectHost>&);

        ~ThumbnailGenerator();

        //! Get the aspect ratio.
        float getAspect() const;

        //! Get a thumbnail.
        ThumbnailRequest getThumbnail(
            const OTIO_NS::RationalTime&,
            int height);

        //! Cancel thumbnail requests.
        void cancelThumbnails(const std::vector<uint64_t>&);

    private:
        void _run();
        void _cancel();

        std::filesystem::path _path;
        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        std::shared_ptr<ImageEffectHost> _host;
        std::shared_ptr<ImageGraph> _graph;
        float _aspect = 1.F;

        struct Request
        {
            uint64_t id = 0;
            int height = 0;
            OTIO_NS::RationalTime time;
            std::promise<std::shared_ptr<dtk::Image> > promise;
        };
        uint64_t _requestId = 0;

        struct Mutex
        {
            std::list<std::shared_ptr<Request> > requests;
            bool stopped = false;
            std::mutex mutex;
        };
        Mutex _mutex;

        struct Thread
        {
            std::map<std::pair<OTIO_NS::RationalTime, int>, std::shared_ptr<dtk::Image> > cache;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        Thread _thread;
    };
}
