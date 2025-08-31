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
    class Viewport : public ftk::IWidget
    {
    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent);

    public:
        virtual ~Viewport();

        //! Create a new widget.
        static std::shared_ptr<Viewport> create(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<App>&,
            const std::shared_ptr<File>&,
            const std::shared_ptr<IWidget>& parent = nullptr);

        //! Get the view position.
        const ftk::V2I& getViewPos() const;

        //! Get the view zoom.
        float getViewZoom() const;

        //! Observe the view position.
        std::shared_ptr<ftk::IObservableValue<ftk::V2I> > observeViewPos() const;

        //! Observe the view zoom.
        std::shared_ptr<ftk::IObservableValue<float> > observeViewZoom() const;

        //! Set the view position and zoom.
        void setViewPosZoom(const ftk::V2I&, float);

        //! Set the view zoom.
        void setViewZoom(float);

        //! Set the view zoom.
        void setViewZoom(float, const ftk::V2I& focus);

        //! Zoom in the view.
        void viewZoomIn(double amount = 2.F);

        //! Zoom out the view.
        void viewZoomOut(double amount = 2.F);

        //! Reset the view zoom.
        void viewZoomReset();

        //! Get whether frame view is enabled.
        bool getFrameView() const;

        //! Observe whether frame view is enabled.
        std::shared_ptr<ftk::IObservableValue<bool> > observeFrameView() const;

        //! Set whether frame view is enabled.
        void setFrameView(bool);

        void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;
        void mouseMoveEvent(ftk::MouseMoveEvent&) override;
        void mousePressEvent(ftk::MouseClickEvent&) override;
        void mouseReleaseEvent(ftk::MouseClickEvent&) override;
        void scrollEvent(ftk::ScrollEvent&) override;

    private:
        ftk::Size2I _getSize() const;
        ftk::TriMesh2F _getMesh(const ftk::Box2I&) const;

        void _frameUpdate();

        std::shared_ptr<ViewModel> _viewModel;
        ftk::Size2I _imageSize;
        std::shared_ptr<ftk::Image> _image;
        ftk::Size2I _bImageSize;
        std::shared_ptr<ftk::Image> _bImage;
        CompareOptions _compareOptions;
        std::shared_ptr<ftk::ObservableValue<ftk::V2I> > _viewPos;
        std::shared_ptr<ftk::ObservableValue<float> > _viewZoom;
        std::shared_ptr<ftk::ObservableValue<bool> > _frameView;
        ViewOptions _options;
        GlobalViewOptions _globalOptions;
        ftk::V2I _viewMousePress;

        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<ftk::Image> > > _imageObserver;
        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > _bObserver;
        std::shared_ptr<ftk::ValueObserver<std::shared_ptr<ftk::Image> > > _bImageObserver;
        std::shared_ptr<ftk::ValueObserver<CompareOptions> > _compareOptionsObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _zoomInObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _zoomOutObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _zoomResetObserver;
        std::shared_ptr<ftk::ValueObserver<bool> > _frameObserver;
        std::shared_ptr<ftk::ValueObserver<ViewOptions> > _optionsObserver;
        std::shared_ptr<ftk::ValueObserver<GlobalViewOptions> > _globalOptionsObserver;
    };
}

