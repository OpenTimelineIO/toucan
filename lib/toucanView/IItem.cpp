// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "IItem.h"

#include "App.h"
#include "File.h"
#include "PlaybackModel.h"

namespace toucan
{
    void IItem::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ItemData& data,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>& object,
        const OTIO_NS::TimeRange& timeRange,
        const std::string& objectName,
        const std::shared_ptr<IWidget>& parent)
    {
        ITimeWidget::_init(context, timeRange, objectName, parent);

        _app = data.app;
        _file = data.file;
        _object = object;

        _timeUnitsObserver = ftk::ValueObserver<TimeUnits>::create(
            data.app->getTimeUnitsModel()->observeTimeUnits(),
            [this](TimeUnits value)
            {
                _timeUnits = value;
                _timeUnitsUpdate();
            });
    }

    IItem::~IItem()
    {}

    const OTIO_NS::SerializableObject::Retainer<OTIO_NS::SerializableObjectWithMetadata>& IItem::getObject() const
    {
        return _object;
    }

    const ftk::Box2I& IItem::getSelectionRect() const
    {
        return _selectionRect;
    }

    bool IItem::isSelected() const
    {
        return _selected;
    }

    void IItem::setSelected(bool value)
    {
        if (value == _selected)
            return;
        _selected = value;
        _setDrawUpdate();
    }

    void IItem::setGeometry(const ftk::Box2I& value)
    {
        ITimeWidget::setGeometry(value);
        _selectionRect = value;
    }

    void IItem::mousePressEvent(ftk::MouseClickEvent& event)
    {
        ITimeWidget::mousePressEvent(event);
        if ((3 == event.button && 0 == event.modifiers) ||
            (1 == event.button && static_cast<int>(ftk::KeyModifier::Super) == event.modifiers))
        {
            event.accept = true;
            _menu = ftk::Menu::create(getContext());
            _buildMenu(_menu);
            auto weak = std::weak_ptr<IItem>(std::dynamic_pointer_cast<IItem>(shared_from_this()));
            _menu->setCloseCallback(
                [weak]
                {
                    if (auto item = weak.lock())
                    {
                        item->_menu.reset();
                    }
                });
            _menu->open(
                getWindow(),
                ftk::Box2I(event.pos.x, event.pos.y, 0, 0));
        }
    }

    void IItem::mouseReleaseEvent(ftk::MouseClickEvent& event)
    {
        ITimeWidget::mouseReleaseEvent(event);
    }

    void IItem::_timeUnitsUpdate()
    {}

    void IItem::_buildMenu(const std::shared_ptr<ftk::Menu>& menu)
    {
        auto action = ftk::Action::create(
            "Go To Start",
            [this]
            {
                if (auto file = _file.lock())
                {
                    file->getPlaybackModel()->setCurrentTime(_timeRange.start_time());
                }
            });
        menu->addAction(action);

        menu->addDivider();

        if (_timeRange.duration().value() > 1.0)
        {
            action = ftk::Action::create(
                "Set In/Out Points",
                [this]
                {
                    if (auto file = _file.lock())
                    {
                        file->getPlaybackModel()->setInOutRange(_timeRange);
                    }
                });
            menu->addAction(action);
        }

        action = ftk::Action::create(
            "Reset In/Out Points",
            [this]
            {
                if (auto file = _file.lock())
                {
                    file->getPlaybackModel()->resetInOutPoints();
                }
            });
        menu->addAction(action);
        bool enabled = true;
        if (auto file = _file.lock())
        {
            enabled =
                file->getPlaybackModel()->getInOutRange() !=
                file->getPlaybackModel()->getTimeRange();
        }
        menu->setEnabled(action, enabled);
    }
}
