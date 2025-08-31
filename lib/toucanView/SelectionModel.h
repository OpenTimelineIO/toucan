// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/core/ObservableList.h>

#include <opentimelineio/timeline.h>

namespace toucan
{
    //! Selection type.
    enum class SelectionType
    {
        All,
        Tracks,
        Clips,
        Markers
    };

    //! Selection item.
    struct SelectionItem
    {
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata> object;
        OTIO_NS::TimeRange timeRange;

        bool operator == (const SelectionItem&) const;
        bool operator != (const SelectionItem&) const;
    };

    //! Get the range of multiple items.
    std::optional<OTIO_NS::TimeRange> getTimeRange(
        const std::vector<SelectionItem>&, 
        const OTIO_NS::RationalTime& startTime,
        double rate);

    //! Selection model.
    class SelectionModel : public std::enable_shared_from_this<SelectionModel>
    {
    public:
        SelectionModel();

        virtual ~SelectionModel();

        //! Get the selection.
        const std::vector<SelectionItem >& getSelection() const;

        //! Observe the selection.
        std::shared_ptr<ftk::IObservableList<SelectionItem> > observeSelection() const;

        //! Set the seldction.
        void setSelection(const std::vector<SelectionItem>&);

        //! Select all of the given type.
        void selectAll(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            SelectionType = SelectionType::All);

        //! Clear the selection.
        void clearSelection();

        //! Invert the selection.
        void invertSelection(const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&);

    private:
        void _getTracks(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            std::vector<SelectionItem>&);
        void _getClips(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            std::vector<SelectionItem>&);
        void _getGaps(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            std::vector<SelectionItem>&);
        void _getMarkers(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            std::vector<SelectionItem>&);

        std::shared_ptr<ftk::ObservableList<SelectionItem> > _selection;
    };
}
