// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FilesModel.h"

#include "PlaybackModel.h"

#include <feather-tk/core/Error.h>
#include <feather-tk/core/Math.h>
#include <feather-tk/core/String.h>

#include <nlohmann/json.hpp>

#include <sstream>

namespace toucan
{
    FEATHER_TK_ENUM_IMPL(
        CompareMode,
        "A",
        "B",
        "Split",
        "Overlay",
        "Horizontal",
        "Vertical");

    bool CompareOptions::operator == (const CompareOptions& other) const
    {
        return
            mode == other.mode &&
            startTime == other.startTime &&
            resize == other.resize;
    }

    bool CompareOptions::operator != (const CompareOptions& other) const
    {
        return !(*this == other);
    }

    FilesModel::FilesModel(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<feather_tk::Settings>& settings,
        const std::shared_ptr<ImageEffectHost>& host) :
        _context(context),
        _settings(settings),
        _host(host)
    {
        CompareOptions compareOptions;
        size_t recentMax = 10;
        std::vector<std::filesystem::path> recent;
        if (_settings)
        {
            try
            {
                nlohmann::json json;
                _settings->get("/FilesModel", json);
                auto i = json.find("CompareMode");
                if (i != json.end() && i->is_string())
                {
                    std::stringstream ss(i->get<std::string>());
                    ss >> compareOptions.mode;
                }
                i = json.find("StartTime");
                if (i != json.end() && i->is_boolean())
                {
                    compareOptions.startTime = i->get<bool>();
                }
                i = json.find("Resize");
                if (i != json.end() && i->is_boolean())
                {
                    compareOptions.resize = i->get<bool>();
                }
                i = json.find("RecentMax");
                if (i != json.end() && i->is_number_unsigned())
                {
                    recentMax = i->get<size_t>();
                }
                i = json.find("Recent");
                if (i != json.end() && i->is_array())
                {
                    for (auto j = i->begin(); j != i->end(); ++j)
                    {
                        if (j->is_string())
                        {
                            recent.push_back(std::filesystem::u8path(j->get<std::string>()));
                        }
                    }
                }
            }
            catch (const std::exception&)
            {}
        }

        _files = feather_tk::ObservableList< std::shared_ptr<File> >::create();
        _add = feather_tk::ObservableValue<int>::create(-1);
        _remove = feather_tk::ObservableValue<int>::create(-1);
        _current = feather_tk::ObservableValue< std::shared_ptr<File> >::create(nullptr);
        _currentIndex = feather_tk::ObservableValue<int>::create(-1);
        _bFile = feather_tk::ObservableValue<std::shared_ptr<File> >::create();
        _bIndex = feather_tk::ObservableValue<int>::create(-1);
        _compareOptions = feather_tk::ObservableValue<CompareOptions>::create(compareOptions);
        _recentFilesModel = feather_tk::RecentFilesModel::create(context);
        _recentFilesModel->setRecentMax(recentMax);
        _recentFilesModel->setRecent(recent);
    }

    FilesModel::~FilesModel()
    {
        if (_settings)
        {
            nlohmann::json json;
            {
                std::stringstream ss;
                ss << _compareOptions->get().mode;
                json["CompareMode"] = ss.str();
            }
            json["StartTime"] = _compareOptions->get().startTime;
            json["Resize"] = _compareOptions->get().resize;
            json["RecentMax"] = _recentFilesModel->getRecentMax();
            nlohmann::json json2;
            for (const auto& path : _recentFilesModel->getRecent())
            {
                json2.push_back(path.u8string());
            }
            json["Recent"] = json2;
            _settings->set("/FilesModel", json);
        }
    }

    void FilesModel::open(const std::filesystem::path& path)
    {
        if (auto context = _context.lock())
        {
            auto file = std::make_shared<File>(context, _host, path);
            auto files = _files->get();
            files.push_back(file);
            _files->setIfChanged(files);

            const int index = files.size() - 1;
            _add->setAlways(index);

            _current->setIfChanged(files[index]);
            _currentIndex->setIfChanged(index);

            _recentFilesModel->addRecent(path);

            _fileUpdate();
        }
    }

    void FilesModel::close()
    {
        close(_currentIndex->get());
    }

    void FilesModel::close(int index)
    {
        auto files = _files->get();
        std::shared_ptr<File> file;
        if (index >= 0 && index < files.size())
        {
            file = *(files.begin() + index);
            files.erase(files.begin() + index);
            _remove->setAlways(index);
            _files->setIfChanged(files);

            int current = std::min(_currentIndex->get(), static_cast<int>(files.size()) - 1);
            _current->setAlways(
                (current >= 0 && current < files.size()) ?
                files[current] :
                nullptr);
            _currentIndex->setAlways(current);

            if (_bIndex->get() == index)
            {
                _bIndex->setIfChanged(-1);
                _bFile->setIfChanged(_getBFile());
            }
        }
        _fileUpdate();
    }

    void FilesModel::closeAll()
    {
        auto files = _files->get();
        for (size_t i = 0; i < files.size(); ++i)
        {
            _remove->setAlways(i);
        }
        _files->setIfChanged({});

        _current->setIfChanged(nullptr);
        _currentIndex->setIfChanged(-1);

        _bFile->setIfChanged(nullptr);
        _bIndex->setIfChanged(-1);

        _currentTimeObserver.reset();
    }

    std::shared_ptr<feather_tk::IObservableList<std::shared_ptr<File> > > FilesModel::observeFiles() const
    {
        return _files;
    }

    std::shared_ptr<feather_tk::IObservableValue<int> > FilesModel::observeAdd() const
    {
        return _add;
    }

    std::shared_ptr<feather_tk::IObservableValue<int> > FilesModel::observeRemove() const
    {
        return _remove;
    }

    std::shared_ptr<feather_tk::IObservableValue<std::shared_ptr<File> > > FilesModel::observeCurrent() const
    {
        return _current;
    }

    std::shared_ptr<feather_tk::IObservableValue<int> > FilesModel::observeCurrentIndex() const
    {
        return _currentIndex;
    }

    void FilesModel::setCurrentIndex(int value)
    {
        const auto& files = _files->get();
        const int index = feather_tk::clamp(value, 0, static_cast<int>(files.size()) - 1);
        _current->setIfChanged(index >= 0 ? files[index] : nullptr);
        _currentIndex->setIfChanged(index);
        _fileUpdate();
    }

    void FilesModel::next()
    {
        const auto& files = _files->get();
        if (!files.empty())
        {
            int index = _currentIndex->get() + 1;
            if (index >= files.size())
            {
                index = 0;
            }
            setCurrentIndex(index);
        }
    }

    void FilesModel::prev()
    {
        const auto& files = _files->get();
        if (!files.empty())
        {
            int index = _currentIndex->get() - 1;
            if (index < 0)
            {
                index = static_cast<int>(files.size()) - 1;
            }
            setCurrentIndex(index);
        }
    }

    const std::shared_ptr<File>& FilesModel::getBFile() const
    {
        return _bFile->get();
    }

    std::shared_ptr<feather_tk::IObservableValue<std::shared_ptr<File> > > FilesModel::observeBFile() const
    {
        return _bFile;
    }

    int FilesModel::getBIndex() const
    {
        return _bIndex->get();
    }

    std::shared_ptr<feather_tk::IObservableValue<int> > FilesModel::observeBIndex() const
    {
        return _bIndex;
    }

    void FilesModel::setBIndex(int value)
    {
        const auto& files = _files->get();
        const int index = feather_tk::clamp(value, -1, static_cast<int>(files.size()) - 1);
        if (_bIndex->setIfChanged(index))
        {
            auto file = _current->get();
            auto bFile = _getBFile();
            if (file && bFile)
            {
                if (bFile != file)
                {
                    bFile->getPlaybackModel()->setPlayback(Playback::Stop);
                }
                _setBTime(file->getPlaybackModel()->getCurrentTime());
            }
            _bFile->setIfChanged(bFile);
        }
    }

    const CompareOptions& FilesModel::getCompareOptions() const
    {
        return _compareOptions->get();
    }

    std::shared_ptr<feather_tk::IObservableValue<CompareOptions> > FilesModel::observeCompareOptions() const
    {
        return _compareOptions;
    }

    void FilesModel::setCompareOptions(const CompareOptions& value)
    {
        if (_compareOptions->setIfChanged(value))
        {
            if (auto file = _current->get())
            {
                _setBTime(file->getPlaybackModel()->getCurrentTime());
            }
        }
    }

    const std::shared_ptr<feather_tk::RecentFilesModel>& FilesModel::getRecentFilesModel() const
    {
        return _recentFilesModel;
    }

    std::shared_ptr<File> FilesModel::_getBFile() const
    {
        std::shared_ptr<File> out;
        const std::vector<std::shared_ptr<File> > files = _files->get();
        const int bIndex = _bIndex->get();
        if (bIndex >= 0 && bIndex < files.size())
        {
            out = files[bIndex];
        }
        return out;
    }

    void FilesModel::_setBTime(const OTIO_NS::RationalTime& value)
    {
        if (auto bFile = _getBFile())
        {
            const OTIO_NS::TimeRange timeRange = bFile->getPlaybackModel()->getTimeRange();
            OTIO_NS::RationalTime tmp(value);
            if (_compareOptions->get().startTime)
            {
                auto file = _current->get();
                const OTIO_NS::RationalTime offset =
                    timeRange.start_time() -
                    file->getPlaybackModel()->getTimeRange().start_time();
                tmp = tmp + offset;
            }
            tmp = tmp.rescaled_to(timeRange.duration()).floor();
            bFile->getPlaybackModel()->setCurrentTime(tmp);
        }
    }

    void FilesModel::_fileUpdate()
    {
        if (auto file = _current->get())
        {
            _currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                file->getPlaybackModel()->observeCurrentTime(),
                [this](const OTIO_NS::RationalTime& value)
                {
                    _setBTime(value);
                });
        }
        else
        {
            _currentTimeObserver.reset();
        }
    }
}
