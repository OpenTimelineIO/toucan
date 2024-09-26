// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#include "Track.h"

namespace toucan
{
    const std::string TrackKind::video = "Video";
    const std::string TrackKind::audio = "Audio";

    Track::Track(const std::string& kind) :
        _kind(kind)
    {}

    Track::~Track()
    {}

    const std::string& Track::getKind() const
    {
        return _kind;
    }
}
