// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Document.h"

#include "PlaybackModel.h"
#include "SelectionModel.h"
#include "ThumbnailGenerator.h"
#include "ViewModel.h"

#include <toucan/Timeline.h>

#include <dtk/core/Math.h>

namespace toucan
{
    Document::Document(
        const std::shared_ptr<dtk::Context>& context,
        const std::shared_ptr<ImageEffectHost>& host,
        const std::filesystem::path& path) :
        _host(host),
        _path(path)
    {
        _timeline = std::make_shared<Timeline>(path.string());        

        _playbackModel = std::make_shared<PlaybackModel>(context);
        const OTIO_NS::TimeRange& timeRange = _timeline->getTimeRange();
        _playbackModel->setTimeRange(timeRange);
        _playbackModel->setCurrentTime(timeRange.start_time());

        _viewModel = std::make_shared<ViewModel>();

        _selectionModel = std::make_shared<SelectionModel>();

        _thumbnailGenerator = std::make_shared<ThumbnailGenerator>(
            _path.parent_path(),
            _timeline,
            _host);

        _currentImage = dtk::ObservableValue<std::shared_ptr<dtk::Image> >::create();

        _rootNode = dtk::ObservableValue<std::shared_ptr<IImageNode> >::create();
        _currentNode = dtk::ObservableValue<std::shared_ptr<IImageNode> >::create();

        ImageGraphOptions graphOptions;
        _graph = std::make_shared<ImageGraph>(
            path.parent_path(),
            _timeline,
            graphOptions);

        _currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
            _playbackModel->observeCurrentTime(),
            [this](const OTIO_NS::RationalTime& value)
            {
                _currentTime = value;
                auto node = _graph->exec(_host, _currentTime);
                _rootNode->setAlways(node);
                _currentNode->setAlways(node);
                _render();
            });
    }

    Document::~Document()
    {}

    const std::filesystem::path& Document::getPath() const
    {
        return _path;
    }

    const std::shared_ptr<Timeline>& Document::getTimeline() const
    {
        return _timeline;
    }

    const std::shared_ptr<PlaybackModel>& Document::getPlaybackModel() const
    {
        return _playbackModel;
    }

    const std::shared_ptr<ViewModel>& Document::getViewModel() const
    {
        return _viewModel;
    }

    const std::shared_ptr<SelectionModel>& Document::getSelectionModel() const
    {
        return _selectionModel;
    }

    const std::shared_ptr<ThumbnailGenerator>& Document::getThumbnailGenerator() const
    {
        return _thumbnailGenerator;
    }

    const IMATH_NAMESPACE::V2i& Document::getImageSize() const
    {
        return _graph->getImageSize();
    }

    std::shared_ptr<dtk::IObservableValue<std::shared_ptr<dtk::Image> > > Document::observeCurrentImage() const
    {
        return _currentImage;
    }

    std::shared_ptr<dtk::IObservableValue<std::shared_ptr<IImageNode> > > Document::observeRootNode() const
    {
        return _rootNode;
    }

    std::shared_ptr<dtk::IObservableValue<std::shared_ptr<IImageNode> > > Document::observeCurrentNode() const
    {
        return _currentNode;
    }
    
    void Document::setCurrentNode(const std::shared_ptr<IImageNode>& value)
    {
        if (_currentNode->setIfChanged(value))
        {
            _render();
        }
    }

    void Document::_render()
    {
        std::shared_ptr<dtk::Image> image;
        if (_currentNode->get())
        {
            const OTIO_NS::TimeRange& timeRange = _playbackModel->getTimeRange();
            _currentNode->get()->setTime(_currentTime - timeRange.start_time());
            _imageBuf = _currentNode->get()->exec();

            const auto& spec = _imageBuf.spec();
            dtk::ImageType imageType = dtk::ImageType::None;
            if (OIIO::TypeDesc::UINT8 == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = dtk::ImageType::L_U8; break;
                case 2: imageType = dtk::ImageType::LA_U8; break;
                case 3: imageType = dtk::ImageType::RGB_U8; break;
                case 4: imageType = dtk::ImageType::RGBA_U8; break;
                default: break;
                }
            }
            else if (OIIO::TypeDesc::UINT16 == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = dtk::ImageType::L_U16; break;
                case 2: imageType = dtk::ImageType::LA_U16; break;
                case 3: imageType = dtk::ImageType::RGB_U16; break;
                case 4: imageType = dtk::ImageType::RGBA_U16; break;
                default: break;
                }
            }
            else if (OIIO::TypeDesc::HALF == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = dtk::ImageType::L_F16; break;
                case 2: imageType = dtk::ImageType::LA_F16; break;
                case 3: imageType = dtk::ImageType::RGB_F16; break;
                case 4: imageType = dtk::ImageType::RGBA_F16; break;
                default: break;
                }
            }
            else if (OIIO::TypeDesc::FLOAT == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = dtk::ImageType::L_F32; break;
                case 2: imageType = dtk::ImageType::LA_F32; break;
                case 3: imageType = dtk::ImageType::RGB_F32; break;
                case 4: imageType = dtk::ImageType::RGBA_F32; break;
                default: break;
                }
            }
            dtk::ImageInfo info(spec.width, spec.height, imageType);
            info.layout.mirror.y = true;
            image = dtk::Image::create(info, reinterpret_cast<uint8_t*>(_imageBuf.localpixels()));
        }
        _currentImage->setIfChanged(image);
    }
}
