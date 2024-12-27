// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Viewport.h"

#include <toucanRender/Util.h>

#include "App.h"
#include "ViewModel.h"

#include <dtk/ui/DrawUtil.h>

namespace toucan
{
    void Viewport::_init(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        IWidget::_init(context, "toucan::Viewport", parent);

        _setMouseHoverEnabled(true);
        _setMousePressEnabled(true, 0, 0);

        _viewModel = file->getViewModel();
        _imageSize.w = file->getImageSize().x;
        _imageSize.h = file->getImageSize().y;
        _viewPos = dtk::ObservableValue<dtk::V2I>::create();
        _viewZoom = dtk::ObservableValue<float>::create(1.F);
        _frameView = dtk::ObservableValue<bool>::create(true);

        _imageObserver = dtk::ValueObserver<std::shared_ptr<dtk::Image> >::create(
            file->observeCurrentImage(),
            [this](const std::shared_ptr<dtk::Image>& value)
            {
                _image = value;
                _setDrawUpdate();
            });

        _bObserver = dtk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeBFile(),
            [this](const std::shared_ptr<File>& value)
            {
                if (value)
                {
                    const auto& imageSize = value->getImageSize();
                    _bImageSize.w = imageSize.x;
                    _bImageSize.h = imageSize.y;
                    _bImageObserver = dtk::ValueObserver<std::shared_ptr<dtk::Image> >::create(
                        value->observeCurrentImage(),
                        [this](const std::shared_ptr<dtk::Image>& value)
                        {
                            _bImage = value;
                            _setDrawUpdate();
                        });
                }
                else
                {
                    _bImageSize = dtk::Size2I();
                    _bImage.reset();
                    _bImageObserver.reset();
                    _setDrawUpdate();
                }
            });

        _compareOptionsObserver = dtk::ValueObserver<CompareOptions>::create(
            app->getFilesModel()->observeCompareOptions(),
            [this](const CompareOptions& value)
            {
                _compareOptions = value;
                _setDrawUpdate();
            });

        _zoomInObserver = dtk::ValueObserver<bool>::create(
            _viewModel->observeZoomIn(),
            [this](bool value)
            {
                if (value)
                {
                    viewZoomIn();
                }
            });

        _zoomOutObserver = dtk::ValueObserver<bool>::create(
            _viewModel->observeZoomOut(),
            [this](bool value)
            {
                if (value)
                {
                    viewZoomOut();
                }
            });

        _zoomResetObserver = dtk::ValueObserver<bool>::create(
            _viewModel->observeZoomReset(),
            [this](bool value)
            {
                if (value)
                {
                    viewZoomReset();
                }
            });

        _frameObserver = dtk::ValueObserver<bool>::create(
            _viewModel->observeFrameView(),
            [this](bool value)
            {
                setFrameView(value);
            });

        _optionsObserver = dtk::ValueObserver<ViewOptions>::create(
            _viewModel->observeOptions(),
            [this](const ViewOptions& value)
            {
                _options = value;
                _setDrawUpdate();
            });

        _globalOptionsObserver = dtk::ValueObserver<GlobalViewOptions>::create(
            app->getGlobalViewModel()->observeOptions(),
            [this](const GlobalViewOptions& value)
            {
                _globalOptions = value;
                _setDrawUpdate();
            });
    }

    Viewport::~Viewport()
    {}

    std::shared_ptr<Viewport> Viewport::create(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<File>& file,
        const std::shared_ptr<dtk::IWidget>& parent)
    {
        auto out = std::shared_ptr<Viewport>(new Viewport);
        out->_init(context, app, file, parent);
        return out;
    }

    const dtk::V2I& Viewport::getViewPos() const
    {
        return _viewPos->get();
    }

    float Viewport::getViewZoom() const
    {
        return _viewZoom->get();
    }

    std::shared_ptr<dtk::IObservableValue<dtk::V2I> > Viewport::observeViewPos() const
    {
        return _viewPos;
    }

    std::shared_ptr<dtk::IObservableValue<float> > Viewport::observeViewZoom() const
    {
        return _viewZoom;
    }

    void Viewport::setViewPosZoom(const dtk::V2I& pos, float zoom)
    {
        setFrameView(false);
        bool changed = _viewPos->setIfChanged(pos);
        changed |= _viewZoom->setIfChanged(zoom);
        if (changed)
        {
            _setDrawUpdate();
        }
    }

    void Viewport::setViewZoom(float value)
    {
        const dtk::Box2I& g = getGeometry();
        const dtk::V2I viewportCenter(g.w() / 2, g.h() / 2);
        setViewZoom(value, _isMouseInside() ? (_getMousePos() - g.min) : viewportCenter);
    }

    void Viewport::setViewZoom(float zoom, const dtk::V2I& focus)
    {
        dtk::V2I pos = _viewPos->get();
        const float zoomPrev = _viewZoom->get();
        pos.x = focus.x + (pos.x - focus.x) * (zoom / zoomPrev);
        pos.y = focus.y + (pos.y - focus.y) * (zoom / zoomPrev);
        setViewPosZoom(pos, zoom);
    }

    void Viewport::viewZoomIn(double amount)
    {
        setViewZoom(_viewZoom->get() * amount);
    }

    void Viewport::viewZoomOut(double amount)
    {
        setViewZoom(_viewZoom->get() / amount);
    }

    void Viewport::viewZoomReset()
    {
        setViewZoom(1.F);
    }

    bool Viewport::getFrameView() const
    {
        return _frameView->get();
    }

    std::shared_ptr<dtk::IObservableValue<bool> > Viewport::observeFrameView() const
    {
        return _frameView;
    }

    void Viewport::setFrameView(bool value)
    {
        if (_frameView->setIfChanged(value))
        {
            _viewModel->setFrameView(value);
            _setDrawUpdate();
        }
    }

    void Viewport::drawEvent(const dtk::Box2I& drawRect, const dtk::DrawEvent& event)
    {
        IWidget::drawEvent(drawRect, event);

        // Draw the background.
        const dtk::Box2I& g = getGeometry();
        switch (_globalOptions.background)
        {
        case ViewBackground::Solid:
            event.render->drawRect(g, _globalOptions.solidColor);
            break;
        case ViewBackground::Checkers:
            event.render->drawColorMesh(dtk::checkers(
                g,
                _globalOptions.checkersColor0,
                _globalOptions.checkersColor1,
                dtk::Size2I(_globalOptions.checkersSize, _globalOptions.checkersSize)));
            break;
        default: break;
        }

        // Setup the view transform.
        if (_frameView->get())
        {
            _frameUpdate();
        }
        dtk::M44F vm;
        vm = vm * dtk::translate(dtk::V3F(g.min.x, g.min.y, 0.F));
        vm = vm * dtk::translate(dtk::V3F(_viewPos->get().x, _viewPos->get().y, 0.F));
        vm = vm * dtk::scale(dtk::V3F(_viewZoom->get(), _viewZoom->get(), 1.F));
        const auto m = event.render->getTransform();
        event.render->setTransform(m * vm);

        // Draw the images.
        IMATH_NAMESPACE::V2i imageSize(_imageSize.w, _imageSize.h);
        if (_image)
        {
            imageSize.x = _image->getWidth();
            imageSize.y = _image->getHeight();
        }
        IMATH_NAMESPACE::V2i bImageSize(_bImageSize.w, _bImageSize.h);
        if (_bImage)
        {
            bImageSize.x = _bImage->getWidth();
            bImageSize.y = _bImage->getHeight();
        }
        IMATH_NAMESPACE::Box2i bImageBox;
        bImageBox.min.x = 0;
        bImageBox.min.y = 0;
        bImageBox.max.x = bImageSize.x - 1;
        bImageBox.max.y = bImageSize.y - 1;
        if (_compareOptions.fitSize)
        {
            bImageBox = fit(imageSize, bImageSize);
        }
        dtk::ImageOptions options;
        options.channelDisplay = _options.channelDisplay;
        options.imageFilters.magnify = dtk::ImageFilter::Nearest;
        switch (_compareOptions.mode)
        {
        case CompareMode::A:
            if (_image)
            {
                event.render->drawImage(
                    _image,
                    _getMesh(dtk::Box2I(0, 0, imageSize.x, imageSize.y)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            break;
        case CompareMode::B:
            if (_bImage)
            {
                event.render->drawImage(
                    _bImage,
                    _getMesh(dtk::Box2I(
                        bImageBox.min.x,
                        bImageBox.min.y,
                        bImageBox.max.x - bImageBox.min.x + 1,
                        bImageBox.max.y - bImageBox.min.y + 1)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            break;
        case CompareMode::Split:
            if (_image)
            {
                dtk::TriMesh2F mesh;
                dtk::Box2I box(0, 0, imageSize.x / 2, imageSize.y);
                mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                mesh.t.push_back(dtk::V2F(_options.flop ? 1.F : 0.F, _options.flip ? 1.F : 0.F));
                mesh.t.push_back(dtk::V2F(_options.flop ? .5F : .5F, _options.flip ? 1.F : 0.F));
                mesh.t.push_back(dtk::V2F(_options.flop ? .5F : .5F, _options.flip ? 0.F : 1.F));
                mesh.t.push_back(dtk::V2F(_options.flop ? 1.F : 0.F, _options.flip ? 0.F : 1.F));
                dtk::Triangle2 triangle;
                triangle.v[0].v = 1;
                triangle.v[1].v = 2;
                triangle.v[2].v = 3;
                triangle.v[0].t = 1;
                triangle.v[1].t = 2;
                triangle.v[2].t = 3;
                mesh.triangles.push_back(triangle);
                triangle.v[0].v = 3;
                triangle.v[1].v = 4;
                triangle.v[2].v = 1;
                triangle.v[0].t = 3;
                triangle.v[1].t = 4;
                triangle.v[2].t = 1;
                mesh.triangles.push_back(triangle);
                event.render->drawImage(
                    _image,
                    mesh,
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            if (_bImage)
            {
                dtk::TriMesh2F mesh;
                dtk::Box2I box(
                    imageSize.x / 2,
                    bImageBox.min.y,
                    (bImageBox.max.x - bImageBox.min.x + 1) / 2,
                    bImageBox.max.y - bImageBox.min.y + 1);
                mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                mesh.t.push_back(dtk::V2F(_options.flop ? .5F : .5F, _options.flip ? 1.F : 0.F));
                mesh.t.push_back(dtk::V2F(_options.flop ? 0.F : 1.F, _options.flip ? 1.F : 0.F));
                mesh.t.push_back(dtk::V2F(_options.flop ? 0.F : 1.F, _options.flip ? 0.F : 1.F));
                mesh.t.push_back(dtk::V2F(_options.flop ? .5F : .5F, _options.flip ? 0.F : 1.F));
                dtk::Triangle2 triangle;
                triangle.v[0].v = 1;
                triangle.v[1].v = 2;
                triangle.v[2].v = 3;
                triangle.v[0].t = 1;
                triangle.v[1].t = 2;
                triangle.v[2].t = 3;
                mesh.triangles.push_back(triangle);
                triangle.v[0].v = 3;
                triangle.v[1].v = 4;
                triangle.v[2].v = 1;
                triangle.v[0].t = 3;
                triangle.v[1].t = 4;
                triangle.v[2].t = 1;
                mesh.triangles.push_back(triangle);
                event.render->drawImage(
                    _bImage,
                    mesh,
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            break;
        case CompareMode::Overlay:
            if (_image && _bImage)
            {
                event.render->drawImage(
                    _image,
                    _getMesh(dtk::Box2I(0, 0, imageSize.x, imageSize.y)),
                    dtk::Color4F(1.F, 1.F, 1.F, .5F),
                    options);
                event.render->drawImage(
                    _bImage,
                    _getMesh(dtk::Box2I(
                        bImageBox.min.x,
                        bImageBox.min.y,
                        bImageBox.max.x - bImageBox.min.x + 1,
                        bImageBox.max.y - bImageBox.min.y + 1)),
                    dtk::Color4F(1.F, 1.F, 1.F, .5F),
                    options);
            }
            else if (_image)
            {
                event.render->drawImage(
                    _image,
                    _getMesh(dtk::Box2I(0, 0, imageSize.x, imageSize.y)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            else if (_bImage)
            {
                event.render->drawImage(
                    _bImage,
                    _getMesh(dtk::Box2I(
                        bImageBox.min.x,
                        bImageBox.min.y,
                        bImageBox.max.x - bImageBox.min.x + 1,
                        bImageBox.max.y - bImageBox.min.y + 1)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            break;
        case CompareMode::Horizontal:
        {
            int x = 0;
            if (_image)
            {
                event.render->drawImage(
                    _image,
                    _getMesh(dtk::Box2I(0, 0, imageSize.x, imageSize.y)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
                x += _image->getWidth();
            }
            if (_bImage)
            {
                event.render->drawImage(
                    _bImage,
                    _getMesh(dtk::Box2I(
                        x + bImageBox.min.x,
                        bImageBox.min.y,
                        bImageBox.max.x - bImageBox.min.x + 1,
                        bImageBox.max.y - bImageBox.min.y + 1)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            break;
        }
        case CompareMode::Vertical:
        {
            int y = 0;
            if (_image)
            {
                event.render->drawImage(
                    _image,
                    _getMesh(dtk::Box2I(0, 0, _image->getWidth(), _image->getHeight())),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
                y += _image->getHeight();
            }
            if (_bImage)
            {
                event.render->drawImage(
                    _bImage,
                    _getMesh(dtk::Box2I(
                        bImageBox.min.x,
                        y + bImageBox.min.y,
                        bImageBox.max.x - bImageBox.min.x + 1,
                        bImageBox.max.y - bImageBox.min.y + 1)),
                    dtk::Color4F(1.F, 1.F, 1.F),
                    options);
            }
            break;
        }
        default: break;
        }

        // Reset the transform.
        event.render->setTransform(m);
    }

    void Viewport::mouseMoveEvent(dtk::MouseMoveEvent& event)
    {
        IWidget::mouseMoveEvent(event);
        if (_isMousePressed())
        {
            event.accept = true;
            const dtk::V2I& mousePressPos = _getMousePressPos();
            _viewPos->setIfChanged(dtk::V2I(
                _viewMousePress.x + (event.pos.x - mousePressPos.x),
                _viewMousePress.y + (event.pos.y - mousePressPos.y)));
            _setDrawUpdate();
        }
    }

    void Viewport::mousePressEvent(dtk::MouseClickEvent& event)
    {
        IWidget::mousePressEvent(event);
        if (0 == event.button && 0 == event.modifiers)
        {
            event.accept = true;
            setFrameView(false);
            _viewMousePress = _viewPos->get();
        }
    }

    void Viewport::mouseReleaseEvent(dtk::MouseClickEvent& event)
    {
        IWidget::mouseReleaseEvent(event);
        event.accept = true;
    }

    void Viewport::scrollEvent(dtk::ScrollEvent& event)
    {
        IWidget::scrollEvent(event);
        if (0 == event.modifiers)
        {
            event.accept = true;
            if (event.value.y > 0)
            {
                viewZoomOut(.9F);
            }
            else if (event.value.y < 0)
            {
                viewZoomIn(.9F);
            }
        }
    }

    dtk::Size2I Viewport::_getSize() const
    {
        dtk::Size2I out = _imageSize;
        const dtk::Size2I bImageSize = _compareOptions.fitSize ? _imageSize : _bImageSize;
        switch (_compareOptions.mode)
        {
        case CompareMode::B:
            out = bImageSize;
            break;
        case CompareMode::Horizontal:
            out.w += bImageSize.w;
            out.h = std::max(out.h, bImageSize.h);
            break;
        case CompareMode::Vertical:
            out.w = std::max(out.w, bImageSize.w);
            out.h += bImageSize.h;
            break;
        default: break;
        }
        return out;
    }

    dtk::TriMesh2F Viewport::_getMesh(const dtk::Box2I& box) const
    {
        dtk::TriMesh2F mesh;
        mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
        mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
        mesh.t.push_back(dtk::V2F(_options.flop ? 1.F : 0.F, _options.flip ? 1.F : 0.F));
        mesh.t.push_back(dtk::V2F(_options.flop ? 0.F : 1.F, _options.flip ? 1.F : 0.F));
        mesh.t.push_back(dtk::V2F(_options.flop ? 0.F : 1.F, _options.flip ? 0.F : 1.F));
        mesh.t.push_back(dtk::V2F(_options.flop ? 1.F : 0.F, _options.flip ? 0.F : 1.F));
        dtk::Triangle2 triangle;
        triangle.v[0].v = 1;
        triangle.v[1].v = 2;
        triangle.v[2].v = 3;
        triangle.v[0].t = 1;
        triangle.v[1].t = 2;
        triangle.v[2].t = 3;
        mesh.triangles.push_back(triangle);
        triangle.v[0].v = 3;
        triangle.v[1].v = 4;
        triangle.v[2].v = 1;
        triangle.v[0].t = 3;
        triangle.v[1].t = 4;
        triangle.v[2].t = 1;
        mesh.triangles.push_back(triangle);
        return mesh;
    }

    void Viewport::_frameUpdate()
    {
        if (_frameView->get())
        {
            const dtk::Box2I& g = getGeometry();
            const dtk::Size2I size = _getSize();
            float zoom = g.w() / static_cast<float>(size.w);
            if (zoom * size.h > g.h())
            {
                zoom = g.h() / static_cast<double>(size.h);
            }
            const dtk::V2I c(size.w / 2, size.h / 2);
            const dtk::V2I pos = dtk::V2I(
                g.w() / 2.F - c.x * zoom,
                g.h() / 2.F - c.y * zoom);

            _viewPos->setIfChanged(pos);
            _viewZoom->setIfChanged(zoom);
        }
    }
}
