// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanView/FilesModel.h>
#include <toucanView/ViewModel.h>

#include <feather-tk/ui/IWidget.h>

#include <feather-tk/core/Image.h>

namespace toucan
{
    class App;
    class File;
    class ViewModel;

    //! Viewport widget.
    class Viewport : public feather_tk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~Viewport();

        //! Create a new widget.
        static std::shared_ptr<Viewport> create(
            const std::shared_ptr<feather_tk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the view position.
        const feather_tk::V2I& getViewPos() const;

        //! Get the view zoom.
        float getViewZoom() const;

        //! Observe the view position.
        std::shared_ptr<feather_tk::IObservableValue<feather_tk::V2I> > observeViewPos() const;

        //! Observe the view zoom.
        std::shared_ptr<feather_tk::IObservableValue<float> > observeViewZoom() const;

        //! Set the view position and zoom.
        void setViewPosZoom(const feather_tk::V2I&, float);

        //! Set the view zoom.
        void setViewZoom(float);

        //! Set the view zoom.
        void setViewZoom(float, const feather_tk::V2I& focus);

        //! Zoom in the view.
        void viewZoomIn(double amount = 2.F);

        //! Zoom out the view.
        void viewZoomOut(double amount = 2.F);

        //! Reset the view zoom.
        void viewZoomReset();

        //! Get whether frame view is enabled.
        bool getFrameView() const;

        //! Observe whether frame view is enabled.
        std::shared_ptr<feather_tk::IObservableValue<bool> > observeFrameView() const;

        //! Set whether frame view is enabled.
        void setFrameView(bool);

        void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;
        void mouseMoveEvent(feather_tk::MouseMoveEvent&) override;
        void mousePressEvent(feather_tk::MouseClickEvent&) override;
        void mouseReleaseEvent(feather_tk::MouseClickEvent&) override;
        void scrollEvent(feather_tk::ScrollEvent&) override;

    private:
        feather_tk::Size2I _getSize() const;
        feather_tk::TriMesh2F _getMesh(const feather_tk::Box2I&) const;

        void _frameUpdate();

        std::shared_ptr<ViewModel> _viewModel;
        feather_tk::Size2I _imageSize;
        std::shared_ptr<feather_tk::Image> _image;
        feather_tk::Size2I _bImageSize;
        std::shared_ptr<feather_tk::Image> _bImage;
        CompareOptions _compareOptions;
        std::shared_ptr<feather_tk::ObservableValue<feather_tk::V2I> > _viewPos;
        std::shared_ptr<feather_tk::ObservableValue<float> > _viewZoom;
        std::shared_ptr<feather_tk::ObservableValue<bool> > _frameView;
        ViewOptions _options;
        GlobalViewOptions _globalOptions;
        feather_tk::V2I _viewMousePress;

        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<feather_tk::Image> > > _imageObserver;
        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<File> > > _bObserver;
        std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<feather_tk::Image> > > _bImageObserver;
        std::shared_ptr<feather_tk::ValueObserver<CompareOptions> > _compareOptionsObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _zoomInObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _zoomOutObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _zoomResetObserver;
        std::shared_ptr<feather_tk::ValueObserver<bool> > _frameObserver;
        std::shared_ptr<feather_tk::ValueObserver<ViewOptions> > _optionsObserver;
        std::shared_ptr<feather_tk::ValueObserver<GlobalViewOptions> > _globalOptionsObserver;
    };
}

