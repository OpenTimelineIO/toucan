// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "SelectionModel.h"

#include <opentimelineio/clip.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/marker.h>

namespace toucan
{
    bool SelectionItem::operator == (const SelectionItem& other) const
    {
        return
            object.value == other.object.value &&
            timeRange == other.timeRange;
    }

    bool SelectionItem::operator != (const SelectionItem& other) const
    {
        return !(*this == other);
    }

    std::optional<OTIO_NS::TimeRange> getTimeRange(
        const std::vector<SelectionItem>& items,
        const OTIO_NS::RationalTime& startTime,
        double rate)
    {
        std::optional<OTIO_NS::TimeRange> out;
        if (!items.empty())
        {
            OTIO_NS::TimeRange timeRange = items.front().timeRange;
            for (size_t i = 1; i < items.size(); ++i)
            {
                timeRange = timeRange.extended_by(items[i].timeRange);
            }
            out = timeRange;
        }
        return out;
    }

    SelectionModel::SelectionModel()
    {
        _selection = ftk::ObservableList<SelectionItem>::create();
    }

    SelectionModel::~SelectionModel()
    {}

    const std::vector<SelectionItem>& SelectionModel::getSelection() const
    {
        return _selection->get();
    }

    std::shared_ptr<ftk::IObservableList<SelectionItem> > SelectionModel::observeSelection() const
    {
        return _selection;
    }

    void SelectionModel::setSelection(const std::vector<SelectionItem>& selection)
    {
        _selection->setIfChanged(selection);
    }

    void SelectionModel::selectAll(
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        SelectionType type)
    {
        std::vector<SelectionItem> objects;
        switch (type)
        {
        case SelectionType::All:
            objects.push_back({ timeline->tracks(), timeline->tracks()->trimmed_range() });
            _getTracks(timeline, objects);
            _getClips(timeline, objects);
            _getGaps(timeline, objects);
            _getMarkers(timeline, objects);
            break;
        case SelectionType::Tracks:
            _getTracks(timeline, objects);
            break;
        case SelectionType::Clips:
            _getClips(timeline, objects);
            break;
        case SelectionType::Markers:
            _getMarkers(timeline, objects);
            break;
        default: break;
        }
        _selection->setIfChanged(objects);
    }

    void SelectionModel::clearSelection()
    {
        _selection->setIfChanged({});
    }

    void SelectionModel::invertSelection(const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline)
    {
        std::vector<SelectionItem> objects;
        objects.push_back({ timeline->tracks(), timeline->tracks()->trimmed_range() });
        _getTracks(timeline, objects);
        _getClips(timeline, objects);
        _getGaps(timeline, objects);
        _getMarkers(timeline, objects);
        const auto& selection = _selection->get();
        auto i = objects.begin();
        while (i != objects.end())
        {
            auto j = std::find(selection.begin(), selection.end(), *i);
            if (j != selection.end())
            {
                i = objects.erase(i);
            }
            else
            {
                ++i;
            }
        }
        _selection->setIfChanged(objects);
    }

    void SelectionModel::_getTracks(
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        std::vector<SelectionItem>& out)
    {
        for (const auto& track : timeline->find_children<OTIO_NS::Track>())
        {
            OTIO_NS::TimeRange timeRange = track->transformed_time_range(track->trimmed_range(), timeline->tracks());
            if (timeline->global_start_time().has_value())
            {
                timeRange = OTIO_NS::TimeRange(
                    timeline->global_start_time().value() + timeRange.start_time(),
                    timeRange.duration());
            }
            out.push_back({
                OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(track),
                timeRange });
        }
    }

    void SelectionModel::_getClips(
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        std::vector<SelectionItem>& out)
    {
        for (const auto& clip : timeline->find_children<OTIO_NS::Clip>())
        {
            OTIO_NS::TimeRange timeRange = clip->transformed_time_range(clip->trimmed_range(), timeline->tracks());
            if (timeline->global_start_time().has_value())
            {
                timeRange = OTIO_NS::TimeRange(
                    timeline->global_start_time().value() + timeRange.start_time(),
                    timeRange.duration());
            }
            out.push_back({
                OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(clip),
                timeRange });
        }
    }

    void SelectionModel::_getGaps(
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        std::vector<SelectionItem>& out)
    {
        for (const auto& gap : timeline->find_children<OTIO_NS::Gap>())
        {
            OTIO_NS::TimeRange timeRange = gap->transformed_time_range(gap->trimmed_range(), timeline->tracks());
            if (timeline->global_start_time().has_value())
            {
                timeRange = OTIO_NS::TimeRange(
                    timeline->global_start_time().value() + timeRange.start_time(),
                    timeRange.duration());
            }
            out.push_back({
                OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(gap),
                timeRange });
        }
    }

    void SelectionModel::_getMarkers(
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        std::vector<SelectionItem>& out)
    {
        //! \bug timeline->find_children<OTIO_NS::Item>() does not include the stack?
        {
            const auto& markers = timeline->tracks()->markers();
            for (const auto& marker : markers)
            {
                out.push_back({
                    OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(marker),
                    marker->marked_range() });
            }
        }
        for (const auto& item : timeline->find_children<OTIO_NS::Item>())
        {
            const auto& markers = item->markers();
            for (const auto& marker : markers)
            {
                OTIO_NS::TimeRange timeRange = item->transformed_time_range(marker->marked_range(), timeline->tracks());
                if (timeline->global_start_time().has_value())
                {
                    timeRange = OTIO_NS::TimeRange(
                        timeline->global_start_time().value() + timeRange.start_time(),
                        timeRange.duration());
                }
                out.push_back({
                    OTIO_NS::dynamic_retainer_cast<OTIO_NS::SerializableObjectWithMetadata>(marker),
                    timeRange });
            }
        }
    }
}
