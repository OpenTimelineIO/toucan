// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ImageGraph.h"

#include "Comp.h"
#include "ImageEffectHost.h"
#include "Read.h"
#include "TimeWarp.h"
#include "Util.h"

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
    }

    ImageGraph::ImageGraph(
        const std::filesystem::path& path,
        const std::shared_ptr<TimelineWrapper>& timelineWrapper,
        const ImageGraphOptions& options) :
        _path(path),
        _timelineWrapper(timelineWrapper),
        _timeRange(timelineWrapper->getTimeRange()),
        _options(options)
    {
        _loadCache.setMax(10);

        // Get the image size from the first video clip.
        for (auto track : _timelineWrapper->getTimeline()->find_children<OTIO_NS::Track>())
        {
            if (OTIO_NS::Track::Kind::video == track->kind())
            {
                for (auto clip : track->find_clips())
                {
                    if (auto externalRef = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                    {
                        auto read = std::make_shared<ReadNode>(
                            _timelineWrapper->getMediaPath(externalRef->target_url()),
                            _timelineWrapper->getMemoryReference(externalRef->target_url()));
                        const auto& spec = read->getSpec();
                        if (spec.width > 0)
                        {
                            _imageSize.x = spec.width;
                            _imageSize.y = spec.height;
                            break;
                        }
                    }
                    else if (auto sequenceRef = dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
                    {
                        auto read = std::make_shared<SequenceReadNode>(
                            _timelineWrapper->getMediaPath(sequenceRef->target_url_base()),
                            sequenceRef->name_prefix(),
                            sequenceRef->name_suffix(),
                            sequenceRef->start_frame(),
                            sequenceRef->frame_step(),
                            sequenceRef->rate(),
                            sequenceRef->frame_zero_padding(),
                            _timelineWrapper->getMemoryReferences());
                        const auto& spec = read->getSpec();
                        if (spec.width > 0)
                        {
                            _imageSize.x = spec.width;
                            _imageSize.y = spec.height;
                            break;
                        }
                    }
                    else if (auto generatorRef = dynamic_cast<OTIO_NS::GeneratorReference*>(clip->media_reference()))
                    {
                        auto parameters = generatorRef->parameters();
                        auto i = parameters.find("size");
                        if (i != parameters.end() && i->second.has_value())
                        {
                            anyToVec(std::any_cast<OTIO_NS::AnyVector>(i->second), _imageSize);
                        }
                    }
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

    std::shared_ptr<IImageNode> ImageGraph::exec(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::RationalTime& time)
    {
        // Set the background color.
        OTIO_NS::AnyDictionary metaData;
        metaData["size"] = vecToAny(_imageSize);
        metaData["color"] = vecToAny(IMATH_NAMESPACE::V4f(0.F, 0.F, 0.F, 1.F));
        std::shared_ptr<IImageNode> node = host->createNode("toucan:Fill", metaData);

        // Loop over the tracks.
        auto stack = _timelineWrapper->getTimeline()->tracks();
        for (const auto& i : stack->children())
        {
            if (auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(i))
            {
                if (track->kind() == OTIO_NS::Track::Kind::video && !track->find_clips().empty())
                {
                    // Process this track.
                    auto trackNode = _track(host, time - _timeRange.start_time(), track);

                    // Get the track effects.
                    const auto& effects = track->effects();
                    if (!effects.empty())
                    {
                        trackNode = _effects(host, effects, trackNode);
                    }

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

        // Get the stack effects.
        const auto& effects = stack->effects();
        if (!effects.empty())
        {
            node = _effects(host, effects, node);
        }

        // Set the time.
        node->setTime(time - _timeRange.start_time());

        return node;
    }

    std::shared_ptr<IImageNode> ImageGraph::_track(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track>& track)
    {
        std::shared_ptr<IImageNode> out;

        // Find the items for the given time.
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
                        trimmedRangeInParent.value(),
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
        if (item)
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

                        auto metaData = prevTransition->metadata();
                        metaData["value"] = value;
                        auto node = host->createNode(
                            prevTransition->transition_type(),
                            metaData);
                        if (!node)
                        {
                            node = host->createNode(
                                "toucan:Dissolve",
                                metaData);
                        }
                        if (node)
                        {
                            auto a = _item(
                                host,
                                prevItem->trimmed_range_in_parent().value(),
                                track->transformed_time(time, prevItem),
                                prevItem);
                            node->setInputs({ a, out });
                            out = node;
                        }
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

                        auto metaData = nextTransition->metadata();
                        metaData["value"] = value;
                        auto node = host->createNode(
                            nextTransition->transition_type(),
                            metaData);
                        if (!node)
                        {
                            node = host->createNode(
                                "toucan:Dissolve",
                                metaData);
                        }
                        if (node)
                        {
                            auto b = _item(
                                host,
                                nextItem->trimmed_range_in_parent().value(),
                                track->transformed_time(time, nextItem),
                                nextItem);
                            node->setInputs({ out, b });
                            out = node;
                        }
                    }
                }
            }
        }

        return out;
    }

    std::shared_ptr<IImageNode> ImageGraph::_item(
        const std::shared_ptr<ImageEffectHost>& host,
        const OTIO_NS::TimeRange& trimmedRangeInParent,
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>& item)
    {
        std::shared_ptr<IImageNode> out;

        OTIO_NS::RationalTime timeOffset =
            trimmedRangeInParent.start_time() -
            item->trimmed_range().start_time();

        if (auto clip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(item))
        {
            // Get the media reference.
            if (auto externalRef = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
            {
                std::shared_ptr<ReadNode> read;
                if (!_loadCache.get(externalRef, read))
                {
                    read = std::make_shared<ReadNode>(
                        _timelineWrapper->getMediaPath(externalRef->target_url()),
                        _timelineWrapper->getMemoryReference(externalRef->target_url()));
                    _loadCache.add(externalRef, read);
                }
                out = read;

                //! \bug Workaround for when the available range does not match
                //! the range in the media.
                const OTIO_NS::TimeRange& timeRange = read->getTimeRange();
                const auto availableOpt = externalRef->available_range();
                if (availableOpt.has_value() &&
                    !availableOpt.value().start_time().strictly_equal(timeRange.start_time()))
                {
                    timeOffset += availableOpt.value().start_time() - timeRange.start_time();
                }
            }
            else if (auto sequenceRef = dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
            {
                auto read = std::make_shared<SequenceReadNode>(
                    _timelineWrapper->getMediaPath(sequenceRef->target_url_base()),
                    sequenceRef->name_prefix(),
                    sequenceRef->name_suffix(),
                    sequenceRef->start_frame(),
                    sequenceRef->frame_step(),
                    sequenceRef->rate(),
                    sequenceRef->frame_zero_padding(),
                    _timelineWrapper->getMemoryReferences());
                out = read;
            }
            else if (auto generatorRef = dynamic_cast<OTIO_NS::GeneratorReference*>(clip->media_reference()))
            {
                out = host->createNode(generatorRef->generator_kind(), generatorRef->parameters());
            }
        }
        else if (auto gap = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(item))
        {
            OTIO_NS::AnyDictionary metaData;
            metaData["size"] = vecToAny(_imageSize);
            out = host->createNode("toucan:Fill", metaData);
        }

        // Get the effects.
        const auto& effects = item->effects();
        if (!effects.empty())
        {
            out = _effects(host, effects, out);
        }
        if (out)
        {
            out->setTimeOffset(timeOffset);
        }

        return out;
    }

    std::shared_ptr<IImageNode> ImageGraph::_effects(
        const std::shared_ptr<ImageEffectHost>& host,
        const std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Effect> >& effects,
        const std::shared_ptr<IImageNode>& input)
    {
        std::shared_ptr<IImageNode> out = input;
        for (const auto& effect : effects)
        {
            if (auto linearTimeWarp = dynamic_cast<OTIO_NS::LinearTimeWarp*>(effect.value))
            {
                auto linearTimeWarpNode = std::make_shared<LinearTimeWarpNode>(
                    static_cast<float>(linearTimeWarp->time_scalar()),
                    std::vector<std::shared_ptr<IImageNode> >{ out });
                out = linearTimeWarpNode;
            }
            else
            {
                if (auto imageEffect = host->createNode(
                    effect->effect_name(),
                    effect->metadata(),
                    { out }))
                {
                    out = imageEffect;
                }
            }
        }
        return out;
    }
}
