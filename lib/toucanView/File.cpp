// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "File.h"

#include "PlaybackModel.h"
#include "SelectionModel.h"
#include "ThumbnailGenerator.h"
#include "ViewModel.h"

#include <toucanRender/TimelineWrapper.h>

#include <feather-tk/core/Math.h>

namespace toucan
{
    File::File(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<ImageEffectHost>& host,
        const std::filesystem::path& path) :
        _host(host),
        _path(path)
    {
        _timelineWrapper = std::make_shared<TimelineWrapper>(path.string());

        _playbackModel = std::make_shared<PlaybackModel>(context);
        const OTIO_NS::TimeRange& timeRange = _timelineWrapper->getTimeRange();
        _playbackModel->setTimeRange(timeRange);
        _playbackModel->setCurrentTime(timeRange.start_time());

        _viewModel = std::make_shared<ViewModel>(context);

        _selectionModel = std::make_shared<SelectionModel>();

        _thumbnailGenerator = std::make_shared<ThumbnailGenerator>(
            context,
            _path.parent_path(),
            _timelineWrapper,
            _host);

        _currentImage = ftk::ObservableValue<std::shared_ptr<ftk::Image> >::create();

        _rootNode = ftk::ObservableValue<std::shared_ptr<IImageNode> >::create();
        _currentNode = ftk::ObservableValue<std::shared_ptr<IImageNode> >::create();

        _graph = std::make_shared<ImageGraph>(
            context,
            path.parent_path(),
            _timelineWrapper);

        _currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
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

    File::~File()
    {}

    const std::filesystem::path& File::getPath() const
    {
        return _path;
    }

    const std::shared_ptr<TimelineWrapper>& File::getTimelineWrapper() const
    {
        return _timelineWrapper;
    }

    const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& File::getTimeline() const
    {
        return _timelineWrapper->getTimeline();
    }

    const std::shared_ptr<PlaybackModel>& File::getPlaybackModel() const
    {
        return _playbackModel;
    }

    const std::shared_ptr<ViewModel>& File::getViewModel() const
    {
        return _viewModel;
    }

    const std::shared_ptr<SelectionModel>& File::getSelectionModel() const
    {
        return _selectionModel;
    }

    const std::shared_ptr<ThumbnailGenerator>& File::getThumbnailGenerator() const
    {
        return _thumbnailGenerator;
    }

    const IMATH_NAMESPACE::V2i& File::getImageSize() const
    {
        return _graph->getImageSize();
    }

    int File::getImageChannels() const
    {
        return _graph->getImageChannels();
    }

    const std::string& File::getImageDataType() const
    {
        return _graph->getImageDataType();
    }

    std::shared_ptr<ftk::IObservableValue<std::shared_ptr<ftk::Image> > > File::observeCurrentImage() const
    {
        return _currentImage;
    }

    std::shared_ptr<ftk::IObservableValue<std::shared_ptr<IImageNode> > > File::observeRootNode() const
    {
        return _rootNode;
    }

    std::shared_ptr<ftk::IObservableValue<std::shared_ptr<IImageNode> > > File::observeCurrentNode() const
    {
        return _currentNode;
    }
    
    void File::setCurrentNode(const std::shared_ptr<IImageNode>& value)
    {
        if (_currentNode->setIfChanged(value))
        {
            _render();
        }
    }

    void File::_render()
    {
        std::shared_ptr<ftk::Image> image;
        if (_currentNode->get())
        {
            const OTIO_NS::TimeRange& timeRange = _playbackModel->getTimeRange();
            _imageBuf = _currentNode->get()->exec(_currentTime - timeRange.start_time());

            const auto& spec = _imageBuf.spec();
            ftk::ImageType imageType = ftk::ImageType::None;
            if (OIIO::TypeDesc::UINT8 == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = ftk::ImageType::L_U8; break;
                case 2: imageType = ftk::ImageType::LA_U8; break;
                case 3: imageType = ftk::ImageType::RGB_U8; break;
                case 4: imageType = ftk::ImageType::RGBA_U8; break;
                default: break;
                }
            }
            else if (OIIO::TypeDesc::UINT16 == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = ftk::ImageType::L_U16; break;
                case 2: imageType = ftk::ImageType::LA_U16; break;
                case 3: imageType = ftk::ImageType::RGB_U16; break;
                case 4: imageType = ftk::ImageType::RGBA_U16; break;
                default: break;
                }
            }
            else if (OIIO::TypeDesc::HALF == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = ftk::ImageType::L_F16; break;
                case 2: imageType = ftk::ImageType::LA_F16; break;
                case 3: imageType = ftk::ImageType::RGB_F16; break;
                case 4: imageType = ftk::ImageType::RGBA_F16; break;
                default: break;
                }
            }
            else if (OIIO::TypeDesc::FLOAT == spec.format)
            {
                switch (spec.nchannels)
                {
                case 1: imageType = ftk::ImageType::L_F32; break;
                case 2: imageType = ftk::ImageType::LA_F32; break;
                case 3: imageType = ftk::ImageType::RGB_F32; break;
                case 4: imageType = ftk::ImageType::RGBA_F32; break;
                default: break;
                }
            }
            ftk::ImageInfo info(spec.width, spec.height, imageType);
            info.layout.mirror.y = true;
            image = ftk::Image::create(info, reinterpret_cast<uint8_t*>(_imageBuf.localpixels()));
        }
        _currentImage->setIfChanged(image);
    }
}
