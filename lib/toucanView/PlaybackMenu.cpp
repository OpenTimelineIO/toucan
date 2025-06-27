// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "PlaybackMenu.h"

#include "App.h"
#include "FilesModel.h"

#include <feather-tk/ui/Action.h>

namespace toucan
{
    void PlaybackMenu::_init(
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        feather_tk::Menu::_init(context, parent);

        _actions["Playback/Stop"] = feather_tk::Action::create(
            "Stop",
            "PlaybackStop",
            feather_tk::Key::K,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setPlayback(Playback::Stop);
                }
            });
        addAction(_actions["Playback/Stop"]);

        _actions["Playback/Forward"] = feather_tk::Action::create(
            "Forward",
            "PlaybackForward",
            feather_tk::Key::L,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->setPlayback(Playback::Forward);
                }
            });
        addAction(_actions["Playback/Forward"]);

        _actions["Playback/Reverse"] = feather_tk::Action::create(
            "Reverse",
            "PlaybackReverse",
            feather_tk::Key::J,
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

        _actions["Playback/Toggle"] = feather_tk::Action::create(
            "Toggle Playback",
            feather_tk::Key::Space,
            0,
            [this]
            {
                if (_file)
                {
                    _file->getPlaybackModel()->togglePlayback();
                }
            });
        addAction(_actions["Playback/Toggle"]);

        _fileObserver = feather_tk::ValueObserver<std::shared_ptr<File> >::create(
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
        const std::shared_ptr<feather_tk::Context>& context,
        const std::shared_ptr<App>& app,
        const std::shared_ptr<feather_tk::IWidget>& parent)
    {
        auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
        out->_init(context, app, parent);
        return out;
    }

    const std::map<std::string, std::shared_ptr<feather_tk::Action> >& PlaybackMenu::getActions() const
    {
        return _actions;
    }

    void PlaybackMenu::_menuUpdate()
    {
        const bool file = _file.get();
        if (file)
        {
            _playbackObserver = feather_tk::ValueObserver<Playback>::create(
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
