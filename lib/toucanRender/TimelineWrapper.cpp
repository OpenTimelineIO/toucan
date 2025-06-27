// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TimelineWrapper.h"

#include "Util.h"

#include <toucanRender/Read.h>

#include <feather-tk/core/String.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_zip_rw.h>

namespace toucan
{
    namespace
    {
        class ZipFile
        {
        public:
            ZipFile(const std::filesystem::path& path)
            {
                mz_zip_reader_create(&handle);
                if (!handle)
                {
                    throw std::runtime_error("Cannot create zip handle");
                }
                int32_t r = mz_zip_reader_open_file(handle, path.string().c_str());
                if (r != 0)
                {
                    throw std::runtime_error("Cannot open zip file: " + path.string());
                }
            }

            ZipFile(const void* data, size_t size)
            {
                mz_zip_reader_create(&handle);
                if (!handle)
                {
                    throw std::runtime_error("Cannot create zip handle");
                }
                int32_t r = mz_zip_reader_open_buffer(
                    handle,
                    reinterpret_cast<uint8_t*>(const_cast<void*>(data)),
                    size,
                    0);
                if (r != 0)
                {
                    throw std::runtime_error("Cannot open memory-mapped zip file");
                }
            }

            ~ZipFile()
            {
                mz_zip_reader_close(handle);
                mz_zip_reader_delete(&handle);
            }

            void* handle = nullptr;
        };
    }

    TimelineWrapper::TimelineWrapper(const std::filesystem::path& path) :
        _path(path)
    {
        const std::string extension = feather_tk::toLower(_path.extension().string());
        if (".otio" == extension)
        {
            OTIO_NS::ErrorStatus errorStatus;
            _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(
                dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(_path.string(), &errorStatus)));
            if (!_timeline)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
        }
        else if (".otiod" == extension)
        {
            _path = _path / "content.otio";
            OTIO_NS::ErrorStatus errorStatus;
            _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(
                dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(_path.string(), &errorStatus)));
            if (!_timeline)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
        }
        else if (".otioz" == extension)
        {
            /*
            // Create a temp directory.
            _tmpPath = makeUniqueTemp();

            // Open the ZIP and extract the .otio file.
            ZipFile zip(path);
            int32_t r = mz_zip_reader_locate_entry(zip.handle, "content.otio", 0);
            if (r != 0)
            {
                throw std::runtime_error("Cannot locate: content.otio");
            }
            std::filesystem::path otioPath = _tmpPath / "content.otio";
            r = mz_zip_reader_entry_save_file(zip.handle, otioPath.string().c_str());
            if (r != 0)
            {
                throw std::runtime_error("Cannot read: content.otio");
            }

            // Read the .otio file.
            OTIO_NS::ErrorStatus errorStatus;
            _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(
                dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(otioPath.string(), &errorStatus)));
            if (!_timeline)
            {
                throw std::runtime_error(errorStatus.full_description);
            }

            // Extract the media files from the ZIP.
            std::filesystem::create_directory(_tmpPath / "media");
            for (const auto& clip : _timeline->find_clips())
            {
                if (auto externalRef = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                {
                    auto split = splitURLProtocol(externalRef->target_url());
                    r = mz_zip_reader_locate_entry(zip.handle, split.second.c_str(), 0);
                    if (r != 0)
                    {
                        throw std::runtime_error("Cannot locate: " + split.second);
                    }
                    std::filesystem::path externalRefPath = _tmpPath / split.second;
                    r = mz_zip_reader_entry_save_file(zip.handle, externalRefPath.string().c_str());
                    if (r != 0)
                    {
                        throw std::runtime_error("Cannot read: " + split.second);
                    }
                }
            }*/

            // Open the ZIP and get the .otio file.
            _memoryMap = std::make_unique<MemoryMap>(path);
            ZipFile zip(_memoryMap->getData(), _memoryMap->getSize());
            int32_t r = mz_zip_reader_locate_entry(zip.handle, "content.otio", 0);
            if (r != 0)
            {
                throw std::runtime_error("Cannot locate: content.otio");
            }
            mz_zip_file* zipInfo = nullptr;
            r = mz_zip_reader_entry_get_info(zip.handle, &zipInfo);
            if (r != 0)
            {
                throw std::runtime_error("Cannot get info: content.otio");
            }
            r = mz_zip_reader_entry_save_buffer_length(zip.handle);
            std::vector<char> buf(r);
            r = mz_zip_reader_entry_save_buffer(zip.handle, buf.data(), r);
            if (r != 0)
            {
                throw std::runtime_error("Cannot save: content.otio");
            }
            buf.push_back(0);

            // Read the .otio file.
            OTIO_NS::ErrorStatus errorStatus;
            _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(
                dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_string(buf.data(), &errorStatus)));
            if (!_timeline)
            {
                throw std::runtime_error(errorStatus.full_description);
            }

            // Create memory references for the media files in the ZIP.
            for (const auto& clip : _timeline->find_clips())
            {
                if (auto externalRef = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                {
                    const std::string url = externalRef->target_url();
                    auto split = splitURLProtocol(url);
                    r = mz_zip_reader_locate_entry(zip.handle, split.second.c_str(), 0);
                    if (r != 0)
                    {
                        throw std::runtime_error("Cannot locate: " + split.second);
                    }
                    r = mz_zip_reader_entry_get_info(zip.handle, &zipInfo);
                    if (r != 0)
                    {
                        throw std::runtime_error("Cannot get info: " + split.second);
                    }
                    if (zipInfo->compression_method != 0)
                    {
                        throw std::runtime_error("Media is not uncompressed: " + split.second);
                    }
                    const size_t headerSize =
                        30 +
                        zipInfo->filename_size +
                        zipInfo->extrafield_size;
                    _memoryReferences[url] = MemoryReference(
                        reinterpret_cast<const uint8_t*>(_memoryMap->getData()) + headerSize + zipInfo->disk_offset,
                        zipInfo->uncompressed_size);
                }
                else if (auto sequenceRef = dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
                {
                    const OTIO_NS::TimeRange timeRange = clip->available_range();
                    for (int64_t frame = timeRange.start_time().value();
                        frame <= timeRange.end_time_inclusive().value();
                        ++frame)
                    {
                        const std::string url = getSequenceFrame(
                            sequenceRef->target_url_base(),
                            sequenceRef->name_prefix(),
                            frame,
                            sequenceRef->frame_zero_padding(),
                            sequenceRef->name_suffix());
                        const auto split = splitURLProtocol(url);
                        r = mz_zip_reader_locate_entry(zip.handle, split.second.c_str(), 0);
                        if (r != 0)
                        {
                            throw std::runtime_error("Cannot locate: " + split.second);
                        }
                        r = mz_zip_reader_entry_get_info(zip.handle, &zipInfo);
                        if (r != 0)
                        {
                            throw std::runtime_error("Cannot get info: " + split.second);
                        }
                        if (zipInfo->compression_method != 0)
                        {
                            throw std::runtime_error("Media is not uncompressed: " + split.second);
                        }
                        const size_t headerSize =
                            30 +
                            zipInfo->filename_size +
                            zipInfo->extrafield_size;
                        _memoryReferences[url] = MemoryReference(
                            reinterpret_cast<const uint8_t*>(_memoryMap->getData()) + headerSize + zipInfo->disk_offset,
                            zipInfo->uncompressed_size);
                    }
                }
            }
        }
        else if (MovieReadNode::hasExtension(path.extension().string()))
        {
            auto read = std::make_shared<MovieReadNode>(path, nullptr);
            _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(new OTIO_NS::Timeline);
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track> track(new OTIO_NS::Track("Video"));
            _timeline->tracks()->append_child(track);
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip> clip(new OTIO_NS::Clip);
            track->append_child(clip);
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::ExternalReference> ref(
                new OTIO_NS::ExternalReference(path.string()));
            clip->set_media_reference(ref);
            clip->set_source_range(read->getTimeRange());
        }
        else if (ImageReadNode::hasExtension(path.extension().string()) ||
            SequenceReadNode::hasExtension(path.extension().string()))
        {
            const auto sequence = getSequence(path);
            const auto split = splitFileNameNumber(sequence.front().stem().string());
            if (split.second.empty())
            {
                auto read = std::make_shared<ImageReadNode>(path, nullptr);
                _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(new OTIO_NS::Timeline);
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track> track(new OTIO_NS::Track("Video"));
                _timeline->tracks()->append_child(track);
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip> clip(new OTIO_NS::Clip);
                track->append_child(clip);
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::ExternalReference> ref(
                    new OTIO_NS::ExternalReference(path.string()));
                clip->set_media_reference(ref);
                clip->set_source_range(OTIO_NS::TimeRange(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::RationalTime(1.0, 24.0)));
            }
            else
            {
                const std::string base = sequence.front().parent_path().string();
                const std::string prefix = split.first;
                const std::string suffix = sequence.front().extension().string();
                const int start = std::atoi(split.second.c_str());
                const int step = 1;
                const double rate = 24.0;
                const int padding = getNumberPadding(split.second);
                auto read = std::make_shared<SequenceReadNode>(base, prefix, suffix, start, 1, rate, padding, nullptr);
                _timeline = OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>(new OTIO_NS::Timeline);
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track> track(new OTIO_NS::Track("Video"));
                _timeline->tracks()->append_child(track);
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip> clip(new OTIO_NS::Clip);
                track->append_child(clip);
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::ImageSequenceReference> ref(
                    new OTIO_NS::ImageSequenceReference(base, prefix, suffix, start, 1, rate, padding));
                clip->set_media_reference(ref);
                clip->set_source_range(OTIO_NS::TimeRange(
                    OTIO_NS::RationalTime(start, rate),
                    OTIO_NS::RationalTime(sequence.size(), rate)));
            }
        }
        else
        {
            throw std::runtime_error("Unrecognized file");
        }

        const auto globalStartTime = _timeline->global_start_time();
        _timeRange = OTIO_NS::TimeRange(
            globalStartTime.has_value() ?
                globalStartTime.value() :
                OTIO_NS::RationalTime(0.0, _timeline->duration().rate()),
            _timeline->duration());
    }

    TimelineWrapper::~TimelineWrapper()
    {
        //if (!_tmpPath.empty())
        //{
        //    std::filesystem::remove_all(_tmpPath);
        //}
    }

    const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& TimelineWrapper::getTimeline() const
    {
        return _timeline;
    }

    const OTIO_NS::TimeRange& TimelineWrapper::getTimeRange() const
    {
        return _timeRange;
    }

    std::string TimelineWrapper::getMediaPath(const std::string& url) const
    {
        std::string out = url;
        if (!_memoryMap.get())
        {
            std::filesystem::path path = splitURLProtocol(url).second;
            if (!path.is_absolute())
            {
                //if (!_tmpPath.empty())
                //{
                //    path = _tmpPath / path;
                //}
                //else
                //{
                    path = _path.parent_path() / path;
                //}
            }
            out = path.string();
        }
        return out;
    }

    const MemoryReferences& TimelineWrapper::getMemoryReferences() const
    {
        return _memoryReferences;
    }

    MemoryReference TimelineWrapper::getMemoryReference(const std::string& url) const
    {
        const auto i = _memoryReferences.find(url);
        return i != _memoryReferences.end() ? i->second : MemoryReference();
    }
}
