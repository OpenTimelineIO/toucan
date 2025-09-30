// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ThumbnailsWidget.h"

#include <toucanRender/Read.h>
#include <toucanRender/TimelineWrapper.h>
#include <toucanRender/Util.h>

#include <ftk/Ui/DrawUtil.h>

namespace toucan
{
    void ThumbnailsWidget::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<TimelineWrapper>& timelineWrapper,
        const OTIO_NS::Item* item,
        const std::shared_ptr<ThumbnailGenerator>& thumbnailGenerator,
        const std::shared_ptr<ftk::LRUCache<std::string, std::shared_ptr<ftk::Image> > >& thumbnailCache,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<IWidget>& parent)
    {
        ITimeWidget::_init(context, timeRange, "toucan::ThumbnailsWidget", parent);
        
        _timelineWrapper = timelineWrapper;
        _item = item;
        _thumbnailGenerator = thumbnailGenerator;
        _thumbnailCache = thumbnailCache;

        _thumbnailAspectRequest = _thumbnailGenerator->getAspect(
            _item,
            timeRange.start_time());
    }
    
    ThumbnailsWidget::~ThumbnailsWidget()
    {}

    std::shared_ptr<ThumbnailsWidget> ThumbnailsWidget::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<TimelineWrapper>& timelineWrapper,
        const OTIO_NS::Item* item,
        const std::shared_ptr<ThumbnailGenerator>& thumbnailGenerator,
        const std::shared_ptr<ftk::LRUCache<std::string, std::shared_ptr<ftk::Image> > >& thumbnailCache,
        const OTIO_NS::TimeRange& timeRange,
        const std::shared_ptr<IWidget>& parent)
    {
        auto out = std::make_shared<ThumbnailsWidget>();
        out->_init(context, timelineWrapper, item, thumbnailGenerator, thumbnailCache, timeRange, parent);
        return out;
    }

    void ThumbnailsWidget::setScale(double value)
    {
        const bool changed = value != _scale;
        ITimeWidget::setScale(value);
        if (changed)
        {
            _cancelThumbnails();
        }
    }

    void ThumbnailsWidget::tickEvent(
        bool parentsVisible,
        bool parentsEnabled,
        const ftk::TickEvent& event)
    {
        ITimeWidget::tickEvent(parentsVisible, parentsEnabled, event);
        if (_thumbnailAspectRequest.valid() &&
            _thumbnailAspectRequest.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            _thumbnailAspect = _thumbnailAspectRequest.get();
        }
        auto i = _thumbnailRequests.begin();
        while (i != _thumbnailRequests.end())
        {
            if (i->future.valid() &&
                i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                const std::string cacheKey = getThumbnailCacheKey(_item, i->time, _size.thumbnailHeight);
                const auto image = i->future.get();
                _thumbnailCache->add(cacheKey, image);
                _setDrawUpdate();
                i = _thumbnailRequests.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    void ThumbnailsWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
    {
        ITimeWidget::sizeHintEvent(event);
        const bool displayScaleChanged = event.displayScale != _size.displayScale;
        if (_size.init || displayScaleChanged)
        {
            _size.init = false;
            _size.displayScale = event.displayScale;
            _size.thumbnailHeight = 2 * event.style->getSizeRole(ftk::SizeRole::SwatchLarge, event.displayScale);
            std::vector<uint64_t> ids;
            for (const auto& request : _thumbnailRequests)
            {
                ids.push_back(request.id);
            }
            _thumbnailRequests.clear();
            _thumbnailGenerator->cancelThumbnails(ids);
        }
        _setSizeHint(ftk::Size2I(0, _size.thumbnailHeight));
    }

    void ThumbnailsWidget::clipEvent(const ftk::Box2I& clipRect, bool clipped)
    {
        ITimeWidget::clipEvent(clipRect, clipped);
        if (clipped)
        {
            _cancelThumbnails();
        }
    }
    
    void ThumbnailsWidget::drawEvent(
        const ftk::Box2I& drawRect,
        const ftk::DrawEvent& event)
    {
        ITimeWidget::drawEvent(drawRect, event);

        const ftk::Box2I& g = getGeometry();
        const int thumbnailWidth = _size.thumbnailHeight * _thumbnailAspect;
        const int y = g.min.y;
        for (int x = g.min.x; x < g.max.x && thumbnailWidth > 0; x += thumbnailWidth)
        {
            const ftk::Box2I g2(x, y, thumbnailWidth, _size.thumbnailHeight);
            if (ftk::intersects(g2, drawRect))
            {
                const OTIO_NS::RationalTime t = posToTime(x);
                const std::string cacheKey = getThumbnailCacheKey(_item, t, _size.thumbnailHeight);
                std::shared_ptr<ftk::Image> image;
                if (_thumbnailCache->get(cacheKey, image))
                {
                    if (image)
                    {
                        const ftk::Box2I g3(x, y, image->getWidth(), image->getHeight());
                        event.render->drawRect(g3, ftk::Color4F(0.F, 0.F, 0.F));
                        event.render->drawImage(image, g3);
                    }
                }
                else
                {
                    const auto j = std::find_if(
                        _thumbnailRequests.begin(),
                        _thumbnailRequests.end(),
                        [this, t](const ThumbnailRequest& request)
                        {
                            return t == request.time && _size.thumbnailHeight == request.height;
                        });
                    if (j == _thumbnailRequests.end())
                    {
                        _thumbnailRequests.push_back(_thumbnailGenerator->getThumbnail(
                            _item,
                            t,
                            _size.thumbnailHeight));
                    }
                }
            }
        }

        std::vector<uint64_t> cancel;
        auto i = _thumbnailRequests.begin();
        while (i != _thumbnailRequests.end())
        {
            const int x = timeToPos(i->time);
            const ftk::Box2I g2(x, y, thumbnailWidth, _size.thumbnailHeight);
            if (!ftk::intersects(g2, drawRect))
            {
                cancel.push_back(i->id);
                i = _thumbnailRequests.erase(i);
            }
            else
            {
                ++i;
            }
        }
        _thumbnailGenerator->cancelThumbnails(cancel);
    }

    void ThumbnailsWidget::_cancelThumbnails()
    {
        std::vector<uint64_t> cancel;
        for (const auto& request : _thumbnailRequests)
        {
            cancel.push_back(request.id);
        }
        _thumbnailRequests.clear();
        _thumbnailGenerator->cancelThumbnails(cancel);
    }
}
