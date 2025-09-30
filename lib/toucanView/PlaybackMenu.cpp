// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "PlaybackMenu.h"

#include "App.h"
#include "FilesModel.h"

#include <ftk/Ui/Action.h>

namespace toucan
{
    void PlaybackMenu::_init(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        ftk::Menu::_init(context, parent);

        _actions["Playback/Stop"] = ftk::Action::create(
            "Stop",
            "PlaybackStop",
            ftk::Key::K,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setPlayback(Playback::Stop);
                }
            });
        addAction(_actions["Playback/Stop"]);

        _actions["Playback/Forward"] = ftk::Action::create(
            "Forward",
            "PlaybackForward",
            ftk::Key::L,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setPlayback(Playback::Forward);
                }
            });
        addAction(_actions["Playback/Forward"]);

        _actions["Playback/Reverse"] = ftk::Action::create(
            "Reverse",
            "PlaybackReverse",
            ftk::Key::J,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setPlayback(Playback::Reverse);
                }
            });
        addAction(_actions["Playback/Reverse"]);

        addDivider();

        _actions["Playback/Toggle"] = ftk::Action::create(
            "Toggle Playback",
            ftk::Key::Space,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->togglePlayback();
                }
            });
        addAction(_actions["Playback/Toggle"]);

        _fileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
            app->getFilesModel()->observeCurrent(),
            [this](const std::shared_ptr<File>& file)
            {
                _file = file;
                _menuUpdate();
            });
    }

    PlaybackMenu::~PlaybackMenu()
    {}

    std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<ftk::IWidget>& parent)
    {
        auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<ftk::Action> >& PlaybackMenu::getActions() const
    {
        return _actions;
    }

    void PlaybackMenu::_menuUpdate()
    {
        const bool file = _file.get();
        if (file)
        {
            _playbackObserver = ftk::ValueObserver<Playback>::create(
                _file->getPlaybackModel()->observePlayback(),
                [this](Playback value)
                {
                    setChecked(_actions["Playback/Stop"], Playback::Stop == value);
                    setChecked(_actions["Playback/Forward"], Playback::Forward == value);
                    setChecked(_actions["Playback/Reverse"], Playback::Reverse == value);
                });
        }
        else
        {
            _playbackObserver.reset();
        }

        setEnabled(_actions["Playback/Stop"], file);
        setEnabled(_actions["Playback/Forward"], file);
        setEnabled(_actions["Playback/Reverse"], file);
        setEnabled(_actions["Playback/Toggle"], file);
    }
}
