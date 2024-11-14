// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Timeline.h"

#include "Util.h"

#include <opentimelineio/externalReference.h>

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

            ~ZipFile()
            {
                mz_zip_reader_close(handle);
                mz_zip_reader_delete(&handle);
            }

            void* handle = nullptr;
        };
    }

    Timeline::Timeline(const std::filesystem::path& path) :
        _path(path)
    {
        const std::string extension = toLower(_path.extension().string());
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
            }
        }
        else
        {
            throw std::runtime_error("Unrecognized file: " + _path.string());
        }

        const auto globalStartTime = _timeline->global_start_time();
        _timeRange = OTIO_NS::TimeRange(
            globalStartTime.has_value() ?
                globalStartTime.value() :
                OTIO_NS::RationalTime(0.0, _timeline->duration().rate()),
            _timeline->duration());
    }

    Timeline::~Timeline()
    {
        if (!_tmpPath.empty())
        {
            std::filesystem::remove_all(_tmpPath);
        }
    }

    const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& Timeline::otio() const
    {
        return _timeline;
    }

    const OTIO_NS::TimeRange& Timeline::getTimeRange() const
    {
        return _timeRange;
    }

    std::filesystem::path Timeline::getMediaPath(const std::string& url) const
    {
        std::filesystem::path path = splitURLProtocol(url).second;
        if (!path.is_absolute())
        {
            if (!_tmpPath.empty())
            {
                path = _tmpPath / path;
            }
            else
            {
                path = _path.parent_path() / path;
            }
        }
        return path;
    }
}
