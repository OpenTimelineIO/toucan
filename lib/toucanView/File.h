// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffectHost.h>
#include <toucanRender/ImageGraph.h>
#include <toucanRender/TimelineWrapper.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Image.h>
#include <ftk/Core/ObservableValue.h>

#include <filesystem>

namespace toucan
{
    class PlaybackModel;
    class SelectionModel;
    class ViewModel;

    //! Timeline file.
    class File : std::enable_shared_from_this<File>
    {
    public:
        File(
            const std::shared_ptr<ftk::Context>&,
            const std::shared_ptr<ImageEffectHost>&,
            const std::filesystem::path&);

        ~File();

        //! Get the path.
        const std::filesystem::path& getPath() const;

        //! Get the timeline wrapper.
        const std::shared_ptr<TimelineWrapper>& getTimelineWrapper() const;

        //! Get the timeline.
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& getTimeline() const;

        //! Get the playback model.
        const std::shared_ptr<PlaybackModel>& getPlaybackModel() const;

        //! Get the view model.
        const std::shared_ptr<ViewModel>& getViewModel() const;

        //! Get the selection model.
        const std::shared_ptr<SelectionModel>& getSelectionModel() const;

        //! Get the image size.
        const IMATH_NAMESPACE::V2i& getImageSize() const;

        //! Get the number of image channels.
        int getImageChannels() const;

        //! Get the image data type.
        const std::string& getImageDataType() const;

        //! Observe the current image.
        std::shared_ptr<ftk::IObservableValue<std::shared_ptr<ftk::Image> > > observeCurrentImage() const;

        //! Observe the root node.
        std::shared_ptr<ftk::IObservableValue<std::shared_ptr<IImageNode> > > observeRootNode() const;

        //! Observe the current node.
        std::shared_ptr<ftk::IObservableValue<std::shared_ptr<IImageNode> > > observeCurrentNode() const;

        //! Set the current node.
        void setCurrentNode(const std::shared_ptr<IImageNode>&);

    private:
        void _render();

        std::shared_ptr<ImageEffectHost> _host;
        std::filesystem::path _path;
        std::shared_ptr<TimelineWrapper> _timelineWrapper;
        std::shared_ptr<PlaybackModel> _playbackModel;
        std::shared_ptr<ViewModel> _viewModel;
        std::shared_ptr<SelectionModel> _selectionModel;
        std::shared_ptr<ftk::ObservableValue<std::shared_ptr<ftk::Image> > > _currentImage;
        OTIO_NS::RationalTime _currentTime;

        std::shared_ptr<ImageGraph> _graph;
        std::shared_ptr<ftk::ObservableValue<std::shared_ptr<IImageNode> > > _rootNode;
        std::shared_ptr<ftk::ObservableValue<std::shared_ptr<IImageNode> > > _currentNode;
        OIIO::ImageBuf _imageBuf;

        std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
    };
}
