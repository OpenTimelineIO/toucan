// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ViewModel.h"

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

#include <nlohmann/json.hpp>

#include <sstream>

namespace toucan
{
    bool ViewOptions::operator == (const ViewOptions& other) const
    {
        return
            flip == other.flip &&
            flop == other.flop &&
            channelDisplay == other.channelDisplay;
    }

    bool ViewOptions::operator != (const ViewOptions& other) const
    {
        return !(*this == other);
    }

    ViewModel::ViewModel(const std::shared_ptr<ftk::Context>& context)
    {
        _zoomIn = ftk::ObservableValue<bool>::create(false);
        _zoomOut = ftk::ObservableValue<bool>::create(false);
        _zoomReset = ftk::ObservableValue<bool>::create(false);
        _frameView = ftk::ObservableValue<bool>::create(true);
        _options = ftk::ObservableValue<ViewOptions>::create();
    }

    ViewModel::~ViewModel()
    {}

    void ViewModel::zoomIn()
    {
        _zoomIn->setAlways(true);
    }

    void ViewModel::zoomOut()
    {
        _zoomOut->setAlways(true);
    }

    void ViewModel::zoomReset()
    {
        _zoomReset->setAlways(true);
    }

    std::shared_ptr<ftk::IObservableValue<bool> > ViewModel::observeZoomIn() const
    {
        return _zoomIn;
    }

    std::shared_ptr<ftk::IObservableValue<bool> > ViewModel::observeZoomOut() const
    {
        return _zoomOut;
    }

    std::shared_ptr<ftk::IObservableValue<bool> > ViewModel::observeZoomReset() const
    {
        return _zoomReset;
    }

    bool ViewModel::getFrameView() const
    {
        return _frameView->get();
    }

    std::shared_ptr<ftk::IObservableValue<bool> > ViewModel::observeFrameView() const
    {
        return _frameView;
    }

    void ViewModel::setFrameView(bool value)
    {
        _frameView->setIfChanged(value);
    }

    const ViewOptions& ViewModel::getOptions() const
    {
        return _options->get();
    }

    std::shared_ptr<ftk::IObservableValue<ViewOptions> > ViewModel::observeOptions() const
    {
        return _options;
    }

    void ViewModel::setOptions(const ViewOptions& value)
    {
        _options->setIfChanged(value);
    }

    FTK_ENUM_IMPL(
        ViewBackground,
        "Solid",
        "Checkers");

    bool GlobalViewOptions::operator == (const GlobalViewOptions& other) const
    {
        return
            hud == other.hud &&
            background == other.background &&
            solidColor == other.solidColor &&
            checkersColor0 == other.checkersColor0 &&
            checkersColor1 == other.checkersColor1 &&
            checkersSize == other.checkersSize;
    }

    bool GlobalViewOptions::operator != (const GlobalViewOptions& other) const
    {
        return !(*this == other);
    }

    GlobalViewModel::GlobalViewModel(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<ftk::Settings>& settings) :
        _settings(settings)
    {
        GlobalViewOptions options;
        try
        {
            nlohmann::json json;
            _settings->get("/GlobalViewModel", json);
            auto i = json.find("HUD");
            if (i != json.end() && i->is_boolean())
            {
                options.hud = i->get<bool>();
            }
            i = json.find("Background");
            if (i != json.end() && i->is_string())
            {
                std::stringstream ss(i->get<std::string>());
                ss >> options.background;
            }
            i = json.find("SolidColor");
            if (i != json.end() && i->is_array() && 4 == i->size())
            {
                options.solidColor = ftk::Color4F(
                    (*i)[0].get<float>(),
                    (*i)[1].get<float>(),
                    (*i)[2].get<float>(),
                    (*i)[3].get<float>());
            }
            i = json.find("CheckersColor0");
            if (i != json.end() && i->is_array() && 4 == i->size())
            {
                options.checkersColor0 = ftk::Color4F(
                    (*i)[0].get<float>(),
                    (*i)[1].get<float>(),
                    (*i)[2].get<float>(),
                    (*i)[3].get<float>());
            }
            i = json.find("CheckersColor1");
            if (i != json.end() && i->is_array() && 4 == i->size())
            {
                options.checkersColor1 = ftk::Color4F(
                    (*i)[0].get<float>(),
                    (*i)[1].get<float>(),
                    (*i)[2].get<float>(),
                    (*i)[3].get<float>());
            }
            i = json.find("CheckersSize");
            if (i != json.end() && i->is_number())
            {
                options.checkersSize = i->get<int>();
            }
        }
        catch (const std::exception&)
        {}

        _options = ftk::ObservableValue<GlobalViewOptions>::create(options);
    }

    GlobalViewModel::~GlobalViewModel()
    {
        nlohmann::json json;
        json["HUD"] = _options->get().hud;
        {
            std::stringstream ss;
            ss << _options->get().background;
            json["Background"] = ss.str();
        }
        json["SolidColor"] =
        {
            _options->get().solidColor.r,
            _options->get().solidColor.g,
            _options->get().solidColor.b,
            _options->get().solidColor.a
        };
        json["CheckersColor0"] =
        {
            _options->get().checkersColor0.r,
            _options->get().checkersColor0.g,
            _options->get().checkersColor0.b,
            _options->get().checkersColor0.a
        };
        json["CheckersColor1"] =
        {
            _options->get().checkersColor1.r,
            _options->get().checkersColor1.g,
            _options->get().checkersColor1.b,
            _options->get().checkersColor1.a
        };
        json["CheckersSize"] = _options->get().checkersSize;
        _settings->set("/GlobalViewModel", json);
    }

    const GlobalViewOptions& GlobalViewModel::getOptions() const
    {
        return _options->get();
    }

    std::shared_ptr<ftk::IObservableValue<GlobalViewOptions> > GlobalViewModel::observeOptions() const
    {
        return _options;
    }

    void GlobalViewModel::setOptions(const GlobalViewOptions& value)
    {
        _options->setIfChanged(value);
    }
}
