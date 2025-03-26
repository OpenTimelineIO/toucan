// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "WindowModel.h"

#include <dtk/ui/Settings.h>
#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <sstream>

namespace toucan
{
    DTK_ENUM_IMPL(
        WindowComponent,
        "ToolBar",
        "Tools",
        "Playback",
        "InfoBar");

    WindowModel::WindowModel(const std::shared_ptr<dtk::Context>& context)
    {
        _settings = context->getSystem<dtk::Settings>();
        std::map<WindowComponent, bool> components =
        {
            { WindowComponent::ToolBar, true },
            { WindowComponent::Tools, true },
            { WindowComponent::Playback, true },
            { WindowComponent::InfoBar, true }
        };
        bool thumbnails = true;
        bool tooltips = true;
        try
        {
            const auto json = std::any_cast<nlohmann::json>(_settings->get("WindowModel"));
            for (auto& i : components)
            {
                std::stringstream ss;
                ss << i.first;
                auto j = json.find(ss.str());
                if (j != json.end() && j->is_boolean())
                {
                    i.second = j->get<bool>();
                }
            }
            auto i = json.find("Thumbnails");
            if (i != json.end() && i->is_boolean())
            {
                thumbnails = i->get<bool>();
            }
            i = json.find("Tooltips");
            if (i != json.end() && i->is_boolean())
            {
                tooltips = i->get<bool>();
            }
        }
        catch (const std::exception&)
        {}

        _components = dtk::ObservableMap<WindowComponent, bool>::create(components);
        _thumbnails = dtk::ObservableValue<bool>::create(thumbnails);
        _tooltips = dtk::ObservableValue<bool>::create(tooltips);
    }

    WindowModel::~WindowModel()
    {
        nlohmann::json json;
        for (const auto i : _components->get())
        {
            std::stringstream ss;
            ss << i.first;
            json[ss.str()] = i.second;
        }
        json["Thumbnails"] = _thumbnails->get();
        json["Tooltips"] = _tooltips->get();
        _settings->set("WindowModel", json);
    }

    const std::map<WindowComponent, bool> WindowModel::getComponents() const
    {
        return _components->get();
    }

    std::shared_ptr<dtk::IObservableMap<WindowComponent, bool> > WindowModel::observeComponents() const
    {
        return _components;
    }

    void WindowModel::setComponents(const std::map<WindowComponent, bool>& value)
    {
        _components->setIfChanged(value);
    }

    bool WindowModel::getComponent(WindowComponent value) const
    {
        return _components->getItem(value);
    }

    void WindowModel::setComponent(WindowComponent component, bool value)
    {
        _components->setItemOnlyIfChanged(component, value);
    }

    bool WindowModel::getThumbnails() const
    {
        return _thumbnails->get();
    }

    std::shared_ptr<dtk::IObservableValue<bool> > WindowModel::observeThumbnails() const
    {
        return _thumbnails;
    }

    void WindowModel::setThumbnails(bool value)
    {
        _thumbnails->setIfChanged(value);
    }

    bool WindowModel::getTooltips() const
    {
        return _tooltips->get();
    }

    std::shared_ptr<dtk::IObservableValue<bool> > WindowModel::observeTooltips() const
    {
        return _tooltips;
    }

    void WindowModel::setTooltips(bool value)
    {
        _tooltips->setIfChanged(value);
    }
}
