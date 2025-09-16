// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ImageGraph.h"

#include "Comp.h"
#include "ImageEffectHost.h"
#include "Read.h"
#include "TimeWarp.h"
#include "TimelineAlgo.h"
#include "Util.h"

#include <feather-tk/core/LogSystem.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/generatorReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/linearTimeWarp.h>

namespace toucan
{
    namespace
    {
        const std::string logPrefix = "toucan::ImageGraph";

        std::string toImageDataType(const OIIO::TypeDesc& value)
        {
            std::string out = "Unknown";
            switch (value.basetype)
            {
            case OIIO::TypeDesc::UINT8:  out = "u8";    break;
            case OIIO::TypeDesc::UINT16: out = "u16";   break;
            case OIIO::TypeDesc::HALF:   out = "half";  break;
            case OIIO::TypeDesc::FLOAT:  out = "float"; break;
            }
            return out;
        }
    }

    ImageGraph::ImageGraph(
        const std::shared_ptr<ftk::Context>& context,
        const std::filesystem::path& path,
        const std::shared_ptr<TimelineWrapper>& timelineWrapper) :
        _context(context),
        _path(path),
        _timelineWrapper(timelineWrapper),
        _timeRange(timelineWrapper->getTimeRange())
    {
        _readCache.setMax(100);

        // Get the image information from the first video clip.
        for (auto clip : getVideoClips(_timelineWrapper->getTimeline()))
        {
            if (auto externalRef = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
            {
                std::shared_ptr<IReadNode> read;
                try
                {
                    read = _timelineWrapper->createReadNode(externalRef);
                }
                catch (const std::exception& e)
                {
                    _context.lock()->getSystem<ftk::LogSystem>()->print(
                        logPrefix,
                        e.what(),
                        ftk::LogType::Error);
                }
                if (read)
                {
                    const auto& spec = read->getSpec();
                    if (spec.width > 0)
                    {
                        _imageSize.x = spec.width;
                        _imageSize.y = spec.height;
                        _imageChannels = spec.nchannels;
                        _imageDataType = toImageDataType(spec.format);
                        break;
                    }
                }
            }
            else if (auto sequenceRef = dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
            {
                std::shared_ptr<IReadNode> read;
                try
                {
                    read = _timelineWrapper->createReadNode(sequenceRef);
                }
                catch (const std::exception& e)
                {
                    _context.lock()->getSystem<ftk::LogSystem>()->print(
                        logPrefix,
                        e.what(),
                        ftk::LogType::Error);
                }
                if (read)
                {
                    const auto& spec = read->getSpec();
                    if (spec.width > 0)
                    {
                        _imageSize.x = spec.width;
                        _imageSize.y = spec.height;
                        _imageChannels = spec.nchannels;
                        _imageDataType = toImageDataType(spec.format);
                        break;
                    }
                }
            }
            else if (auto generatorRef = dynamic_cast<OTIO_NS::GeneratorReference*>(clip->media_reference()))
            {
                auto parameters = generatorRef->parameters();
                auto i = parameters.find("size");
                if (i != parameters.end() && i->second.has_value())
                {
                    anyToVec(std::any_cast<OTIO_NS::AnyVector>(i->second), _imageSize);
                    //! \bug Hard coded:
                    _imageChannels = 4;
                    _imageDataType = toImageDataType(OIIO::TypeDesc::UINT8);
                    break;
                }
            }
        }
    }

    ImageGraph::~ImageGraph()
    {}

    const IMATH_NAMESPACE::V2i& ImageGraph::getImageSize() const
    {
        return _imageSize;
    }

    int ImageGraph::getImageChannels() const
    {
        return _imageChannels;
    }

    const std::string& ImageGraph::getImageDataType() const
    {
        return _imageDataType;
    }

    std::shared_ptr<IImageNode> ImageGraph::exec(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::RationalTime& time)
    {
        // Set the background color.
        OTIO_NS::AnyDictionary metaData;
        metaData["size"] = vecToAny(_imageSize);
        metaData["color"] = vecToAny(IMATH_NAMESPACE::V4f(0.F, 0.F, 0.F, 1.F));
        std::shared_ptr<IImageNode> node = host->createNode(metaData, "toucan:Fill");

        // Apply time warps.
        auto stack = _timelineWrapper->getTimeline()->tracks();
        const auto& stackEffects = stack->effects();
        OTIO_NS::RationalTime t = time - _timeRange.start_time();
        t = _timeWarps(t, stack->available_range(), stackEffects);

        // Loop over the tracks.
        for (const auto& i : stack->children())
        {
            if (auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(i))
            {
                if (track->kind() == OTIO_NS::Track::Kind::video && !track->find_clips().empty())
                {
                    // Apply time warps.
                    const auto& trackEffects = track->effects();
                    OTIO_NS::RationalTime t2 = t;
                    if (!trackEffects.empty())
                    {
                        t2 = _timeWarps(t2, track->available_range(), trackEffects);
                    }

                    // Process this track.
                    auto trackNode = _track(host, t2, track);

                    // Add the track effects.
                    trackNode = _effects(host, t2, trackEffects, trackNode);

                    // Composite this track over the previous track.
                    std::vector<std::shared_ptr<IImageNode> > nodes;
                    if (trackNode)
                    {
                        nodes.push_back(trackNode);
                    }
                    if (node)
                    {
                        nodes.push_back(node);
                    }
                    auto comp = std::make_shared<CompNode>(nodes);
                    comp->setPremult(true);
                    node = comp;
                }
            }
        }

        // Add the stack effects.
        node = _effects(host, t, stackEffects, node);

        return node;
    }

    std::shared_ptr<IImageNode> ImageGraph::_track(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>& track)
    {
        std::shared_ptr<IImageNode> out;

        // Find the item for the given time. The previous and next items are
        // also tracked for handling transitions.
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item> item;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Composable> prev;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Composable> prev2;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Composable> next;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Composable> next2;
        const auto& children = track->children();
        for (size_t i = 0; i < children.size(); ++i)
        {
            if ((item = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(children[i])))
            {
                const auto trimmedRangeInParent = item->trimmed_range_in_parent();
                if (trimmedRangeInParent.has_value() && trimmedRangeInParent.value().contains(time))
                {
                    out = _item(
                        host,
                        track->transformed_time(time, item),
                        item);
                    if (i > 0)
                    {
                        prev = children[i - 1];
                    }
                    if (i > 1)
                    {
                        prev2 = children[i - 2];
                    }
                    if (i < (children.size() - 1))
                    {
                        next = children[i + 1];
                    }
                    if (children.size() > 1 && i < (children.size() - 2))
                    {
                        next2 = children[i + 2];
                    }
                    break;
                }
            }
        }

        // Handle transitions.
        if (item && out)
        {
            if (auto prevTransition = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Transition>(prev))
            {
                const auto trimmedRangeInParent = prevTransition->trimmed_range_in_parent();
                if (trimmedRangeInParent.has_value() && trimmedRangeInParent.value().contains(time))
                {
                    if (auto prevItem = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(prev2))
                    {
                        const double value =
                            (time - trimmedRangeInParent.value().start_time()).value() /
                            trimmedRangeInParent.value().duration().value();

                        auto a = _item(
                            host,
                            track->transformed_time(time, prevItem),
                            prevItem);

                        auto metaData = prevTransition->metadata();
                        metaData["value"] = value;
                        auto node = host->createNode(
                            metaData,
                            prevTransition->transition_type(),
                            { a, out });
                        if (!node)
                        {
                            node = host->createNode(
                                metaData,
                                "toucan:Dissolve",
                                { a, out });
                        }
                        out = node;
                    }
                }
            }
            if (auto nextTransition = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Transition>(next))
            {
                const auto trimmedRangeInParent = nextTransition->trimmed_range_in_parent();
                if (trimmedRangeInParent.has_value() && trimmedRangeInParent.value().contains(time))
                {
                    if (auto nextItem = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(next2))
                    {
                        const double value =
                            (time - trimmedRangeInParent.value().start_time()).value() /
                            trimmedRangeInParent.value().duration().value();

                        auto b = _item(
                            host,
                            track->transformed_time(time, nextItem),
                            nextItem);

                        auto metaData = nextTransition->metadata();
                        metaData["value"] = value;
                        auto node = host->createNode(
                            metaData,
                            nextTransition->transition_type(),
                            { out, b });
                        if (!node)
                        {
                            node = host->createNode(
                                metaData,
                                "toucan:Dissolve",
                                { out, b });
                        }
                        out = node;
                    }
                }
            }
        }

        return out;
    }

    std::shared_ptr<IImageNode> ImageGraph::_item(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>& item)
    {
        std::shared_ptr<IImageNode> out;

        OTIO_NS::RationalTime t = time;

        // Apply time warps.
        const auto& effects = item->effects();
        t = _timeWarps(t, item->available_range(), effects);

        // Get the media.
        if (auto clip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(item))
        {
            auto mediaRef = clip->media_reference();
            if (auto externalRef = dynamic_cast<OTIO_NS::ExternalReference*>(mediaRef))
            {
                std::shared_ptr<IReadNode> read;
                if (!_readCache.get(externalRef, read))
                {
                    try
                    {
                        read = _timelineWrapper->createReadNode(externalRef);
                        _readCache.add(externalRef, read);
                    }
                    catch (const std::exception& e)
                    {
                        _context.lock()->getSystem<ftk::LogSystem>()->print(
                            logPrefix,
                            e.what(),
                            ftk::LogType::Error);
                    }
                }
                if (read)
                {
                    //! \bug Workaround for files that are missing timecode.
                    if (clip->available_range().start_time() !=
                        read->getTimeRange().start_time())
                    {
                        t -= clip->available_range().start_time();
                    }

                    read->setTime(t);
                }
                out = read;
            }
            else if (auto sequenceRef = dynamic_cast<OTIO_NS::ImageSequenceReference*>(mediaRef))
            {
                std::shared_ptr<IReadNode> read;
                if (!_readCache.get(sequenceRef, read))
                {
                    try
                    {
                        read = _timelineWrapper->createReadNode(sequenceRef);
                        _readCache.add(sequenceRef, read);
                    }
                    catch (const std::exception& e)
                    {
                        _context.lock()->getSystem<ftk::LogSystem>()->print(
                            logPrefix,
                            e.what(),
                            ftk::LogType::Error);
                    }
                }
                if (read)
                {
                    read->setTime(t);
                }
                out = read;
            }
            else if (auto generatorRef = dynamic_cast<OTIO_NS::GeneratorReference*>(mediaRef))
            {
                out = host->createNode(
                    generatorRef->parameters(),
                    generatorRef->generator_kind());
            }
        }
        else if (auto gap = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(item))
        {
            OTIO_NS::AnyDictionary metaData;
            metaData["size"] = vecToAny(_imageSize);
            out = host->createNode(metaData, "toucan:Fill");
        }

        // Add the effects.
        out = _effects(host, t, effects, out);

        return out;
    }

    OTIO_NS::RationalTime ImageGraph::_timeWarps(
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::TimeRange& timeRange,
        const std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Effect> >& effects)
    {
        OTIO_NS::RationalTime out = time;
        for (const auto& effect : effects)
        {
            if (auto linearTimeWarp = dynamic_cast<OTIO_NS::LinearTimeWarp*>(effect.value))
            {
                const double s = linearTimeWarp->time_scalar();
                out = OTIO_NS::RationalTime(
                    (out - timeRange.start_time()).value() * s,
                    time.rate()).round();
            }
        }
        return out;
    }

    std::shared_ptr<IImageNode> ImageGraph::_effects(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::RationalTime& time,
        const std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Effect> >& effects,
        const std::shared_ptr<IImageNode>& input)
    {
        std::shared_ptr<IImageNode> out = input;
        for (const auto& effect : effects)
        {
            if (auto imageEffect = host->createNode(
                effect->metadata(),
                effect->effect_name(),
                { out }))
            {
                out = imageEffect;
            }
        }
        return out;
    }
}
