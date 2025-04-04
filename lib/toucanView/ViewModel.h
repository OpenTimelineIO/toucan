// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <dtk/ui/Settings.h>
#include <dtk/core/Context.h>
#include <dtk/core/ObservableValue.h>
#include <dtk/core/RenderOptions.h>

#include <opentimelineio/timeline.h>

namespace toucan
{
    //! View options.
    struct ViewOptions
    {
        bool flip = false;
        bool flop = false;
        dtk::ChannelDisplay channelDisplay = dtk::ChannelDisplay::Color;

        bool operator == (const ViewOptions&) const;
        bool operator != (const ViewOptions&) const;
    };

    //! View model.
    class ViewModel : public std::enable_shared_from_this<ViewModel>
    {
    public:
        ViewModel(const std::shared_ptr<dtk::Context>&);

        virtual ~ViewModel();

        //! Zoom in.
        void zoomIn();

        //! Zoom out.
        void zoomOut();

        //! Reset the zoom.
        void zoomReset();

        //! Observe the zoom in.
        std::shared_ptr<dtk::IObservableValue<bool> > observeZoomIn() const;

        //! Observe the zoom out.
        std::shared_ptr<dtk::IObservableValue<bool> > observeZoomOut() const;

        //! Observe the zoom reset.
        std::shared_ptr<dtk::IObservableValue<bool> > observeZoomReset() const;

        //! Get whether frame view is enabled.
        bool getFrameView() const;

        //! Observe whether frame view is enabled.
        std::shared_ptr<dtk::IObservableValue<bool> > observeFrameView() const;

        //! Set whether frame view is enabled.
        void setFrameView(bool);

        //! Get the view options.
        const ViewOptions& getOptions() const;

        //! Observe the view options.
        std::shared_ptr<dtk::IObservableValue<ViewOptions> > observeOptions() const;

        //! Set the view options.
        void setOptions(const ViewOptions&);

    private:
        std::shared_ptr<dtk::ObservableValue<bool> > _zoomIn;
        std::shared_ptr<dtk::ObservableValue<bool> > _zoomOut;
        std::shared_ptr<dtk::ObservableValue<bool> > _zoomReset;
        std::shared_ptr<dtk::ObservableValue<bool> > _frameView;
        std::shared_ptr<dtk::ObservableValue<ViewOptions> > _options;
    };

    //! View background options.
    enum ViewBackground
    {
        Solid,
        Checkers,

        Count,
        First = Solid
    };
    DTK_ENUM(ViewBackground);

    //! Global view options.
    struct GlobalViewOptions
    {
        bool hud = false;
        ViewBackground background = ViewBackground::Solid;
        dtk::Color4F solidColor = dtk::Color4F(0.F, 0.F, 0.F, 1.F);
        dtk::Color4F checkersColor0 = dtk::Color4F(0.F, 0.F, 0.F, 1.F);
        dtk::Color4F checkersColor1 = dtk::Color4F(1.F, 1.F, 1.F, 1.F);
        int checkersSize = 50;

        bool operator == (const GlobalViewOptions&) const;
        bool operator != (const GlobalViewOptions&) const;
    };

    //! Global view model.
    class GlobalViewModel : public std::enable_shared_from_this<GlobalViewModel>
    {
    public:
        GlobalViewModel(const std::shared_ptr<dtk::Context>&);

        virtual ~GlobalViewModel();

        //! Get the view options.
        const GlobalViewOptions& getOptions() const;

        //! Observe the view options.
        std::shared_ptr<dtk::IObservableValue<GlobalViewOptions> > observeOptions() const;

        //! Set the view options.
        void setOptions(const GlobalViewOptions&);

    private:
        std::shared_ptr<dtk::Settings> _settings;
        std::shared_ptr<dtk::ObservableValue<GlobalViewOptions> > _options;
    };
}
