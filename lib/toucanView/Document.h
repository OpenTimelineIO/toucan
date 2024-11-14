// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucan/ImageEffectHost.h>
#include <toucan/ImageGraph.h>
#include <toucan/Timeline.h>

#include <dtk/core/Context.h>
#include <dtk/core/Image.h>
#include <dtk/core/ObservableValue.h>

#include <opentimelineio/timeline.h>

#include <filesystem>

namespace toucan
{
    class PlaybackModel;
    class SelectionModel;
    class ThumbnailGenerator;
    class ViewModel;

    class Document : std::enable_shared_from_this<Document>
    {
    public:
        Document(
            const std::shared_ptr<dtk::Context>&,
            const std::shared_ptr<ImageEffectHost>&,
            const std::filesystem::path&);

        ~Document();

        const std::filesystem::path& getPath() const;

        const std::shared_ptr<Timeline>& getTimeline() const;

        const std::shared_ptr<PlaybackModel>& getPlaybackModel() const;
        const std::shared_ptr<ViewModel>& getViewModel() const;
        const std::shared_ptr<SelectionModel>& getSelectionModel() const;
        const std::shared_ptr<ThumbnailGenerator>& getThumbnailGenerator() const;

        const IMATH_NAMESPACE::V2i& getImageSize() const;

        std::shared_ptr<dtk::IObservableValue<std::shared_ptr<dtk::Image> > > observeCurrentImage() const;

        std::shared_ptr<dtk::IObservableValue<std::shared_ptr<IImageNode> > > observeRootNode() const;
        std::shared_ptr<dtk::IObservableValue<std::shared_ptr<IImageNode> > > observeCurrentNode() const;
        void setCurrentNode(const std::shared_ptr<IImageNode>&);

    private:
        void _render();

        std::shared_ptr<ImageEffectHost> _host;
        std::filesystem::path _path;
        std::shared_ptr<Timeline> _timeline;
        std::shared_ptr<PlaybackModel> _playbackModel;
        std::shared_ptr<ViewModel> _viewModel;
        std::shared_ptr<SelectionModel> _selectionModel;
        std::shared_ptr<ThumbnailGenerator> _thumbnailGenerator;
        std::shared_ptr<dtk::ObservableValue<std::shared_ptr<dtk::Image> > > _currentImage;
        OTIO_NS::RationalTime _currentTime;

        std::shared_ptr<ImageGraph> _graph;
        std::shared_ptr<dtk::ObservableValue<std::shared_ptr<IImageNode> > > _rootNode;
        std::shared_ptr<dtk::ObservableValue<std::shared_ptr<IImageNode> > > _currentNode;
        OIIO::ImageBuf _imageBuf;

        std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
    };
}
