// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/ThumbnailGenerator.h>
#include <toucanView/TimeLayout.h>
#include <toucanView/TimeUnitsModel.h>

#include <feather-tk/ui/IWidget.h>
#include <feather-tk/ui/Menu.h>
#include <feather-tk/core/LRUCache.h>

#include <opentimelineio/timeline.h>

namespace toucan
{
    class App;
    class File;

    //! Timeline item data.
    struct ItemData
    {
        std::shared_ptr<App> app;
        std::shared_ptr<File> file;
        std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;
        std::shared_ptr<feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > > thumbnailCache;
    };

    //! Base class for timeline items.
    class IItem : public ITimeWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const ItemData&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>&,
            const OTIO_NS::TimeRange&,
            const std::string& objectName,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~IItem() = 0;

        //! Get the OTIO object.
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>& getObject() const;
        
        //! Get the item selection rectangle.
        const feather_tk::Box2I& getSelectionRect() const;

        //! Get whether the item is selected.
        bool isSelected() const;

        //! Set whether the item is selected.
        void setSelected(bool);

        void setGeometry(const feather_tk::Box2I&) override;
        void mousePressEvent(feather_tk::MouseClickEvent&) override;
        void mouseReleaseEvent(feather_tk::MouseClickEvent&) override;

    protected:
        virtual void _timeUnitsUpdate();
        virtual void _buildMenu(const std::shared_ptr<feather_tk::Menu>&);

        std::weak_ptr<App> _app;
        std::weak_ptr<File> _file;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata> _object;
        TimeUnits _timeUnits = TimeUnits::First;
        feather_tk::Box2I _selectionRect;
        bool _selected = false;
        std::shared_ptr<feather_tk::Menu> _menu;

    private:
        std::shared_ptr<feather_tk::ValueObserver<TimeUnits> > _timeUnitsObserver;
    };
}
