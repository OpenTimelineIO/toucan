// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <dtk/ui/Settings.h>
#include <dtk/core/Context.h>
#include <dtk/core/ObservableMap.h>
#include <dtk/core/ObservableValue.h>

namespace toucan
{
    //! Window components.
    enum class WindowComponent
    {
        ToolBar,
        Tools,
        Playback,
        InfoBar,

        Count,
        First = ToolBar
    };
    DTK_ENUM(WindowComponent);

    //! Window model.
    class WindowModel : public std::enable_shared_from_this<WindowModel>
    {
    public:
        WindowModel(const std::shared_ptr<dtk::Context>&);

        virtual ~WindowModel();

        //! Get the window components.
        const std::map<WindowComponent, bool> getComponents() const;

        //! Observe the window components.
        std::shared_ptr<dtk::IObservableMap<WindowComponent, bool> > observeComponents() const;

        //! Set the window components.
        void setComponents(const std::map<WindowComponent, bool>&);

        //! Get a window component.
        bool getComponent(WindowComponent) const;

        //! Set a window component.
        void setComponent(WindowComponent, bool);

        //! Get whether thumbnails are enabled.
        bool getThumbnails() const;

        //! Observe whether thumbnails are enabled.
        std::shared_ptr<dtk::IObservableValue<bool> > observeThumbnails() const;

        //! Set whether thumbnails are enabled.
        void setThumbnails(bool);

        //! Get whether tooltips are enabled.
        bool getTooltips() const;

        //! Observe whether tooltips are enabled.
        std::shared_ptr<dtk::IObservableValue<bool> > observeTooltips() const;

        //! Set whether tooltips are enabled.
        void setTooltips(bool);

    private:
        std::shared_ptr<dtk::Settings> _settings;
        std::shared_ptr<dtk::ObservableMap<WindowComponent, bool> > _components;
        std::shared_ptr<dtk::ObservableValue<bool> > _thumbnails;
        std::shared_ptr<dtk::ObservableValue<bool> > _tooltips;
    };
}
