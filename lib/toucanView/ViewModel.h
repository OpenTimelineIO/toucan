// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <feather-tk/ui/Settings.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/ObservableValue.h>
#include <feather-tk/core/RenderOptions.h>

#include <opentimelineio/timeline.h>

namespace toucan
{
    //! View options.
    struct ViewOptions
    {
        bool flip = false;
        bool flop = false;
        ftk::ChannelDisplay channelDisplay = ftk::ChannelDisplay::Color;

        bool operator == (const ViewOptions&) const;
        bool operator != (const ViewOptions&) const;
    };

    //! View model.
    class ViewModel : public std::enable_shared_from_this<ViewModel>
    {
    public:
        ViewModel(const std::shared_ptr<ftk::Context>&);

        virtual ~ViewModel();

        //! Zoom in.
        void zoomIn();

        //! Zoom out.
        void zoomOut();

        //! Reset the zoom.
        void zoomReset();

        //! Observe the zoom in.
        std::shared_ptr<ftk::IObservableValue<bool> > observeZoomIn() const;

        //! Observe the zoom out.
        std::shared_ptr<ftk::IObservableValue<bool> > observeZoomOut() const;

        //! Observe the zoom reset.
        std::shared_ptr<ftk::IObservableValue<bool> > observeZoomReset() const;

        //! Get whether frame view is enabled.
        bool getFrameView() const;

        //! Observe whether frame view is enabled.
        std::shared_ptr<ftk::IObservableValue<bool> > observeFrameView() const;

        //! Set whether frame view is enabled.
        void setFrameView(bool);

        //! Get the view options.
        const ViewOptions& getOptions() const;

        //! Observe the view options.
        std::shared_ptr<ftk::IObservableValue<ViewOptions> > observeOptions() const;

        //! Set the view options.
        void setOptions(const ViewOptions&);

    private:
        std::shared_ptr<ftk::ObservableValue<bool> > _zoomIn;
        std::shared_ptr<ftk::ObservableValue<bool> > _zoomOut;
        std::shared_ptr<ftk::ObservableValue<bool> > _zoomReset;
        std::shared_ptr<ftk::ObservableValue<bool> > _frameView;
        std::shared_ptr<ftk::ObservableValue<ViewOptions> > _options;
    };

    //! View background options.
    enum ViewBackground
    {
        Solid,
        Checkers,

        Count,
        First = Solid
    };
    FTK_ENUM(ViewBackground);

    //! Global view options.
    struct GlobalViewOptions
    {
        bool hud = false;
        ViewBackground background = ViewBackground::Solid;
        ftk::Color4F solidColor = ftk::Color4F(0.F, 0.F, 0.F, 1.F);
        ftk::Color4F checkersColor0 = ftk::Color4F(0.F, 0.F, 0.F, 1.F);
        ftk::Color4F checkersColor1 = ftk::Color4F(1.F, 1.F, 1.F, 1.F);
        int checkersSize = 50;

        bool operator == (const GlobalViewOptions&) const;
        bool operator != (const GlobalViewOptions&) const;
    };

    //! Global view model.
    class GlobalViewModel : public std::enable_shared_from_this<GlobalViewModel>
    {
    public:
        GlobalViewModel(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<ftk::Settings>&);

        virtual ~GlobalViewModel();

        //! Get the view options.
        const GlobalViewOptions& getOptions() const;

        //! Observe the view options.
        std::shared_ptr<ftk::IObservableValue<GlobalViewOptions> > observeOptions() const;

        //! Set the view options.
        void setOptions(const GlobalViewOptions&);

    private:
        std::shared_ptr<ftk::Settings> _settings;
        std::shared_ptr<ftk::ObservableValue<GlobalViewOptions> > _options;
    };
}
