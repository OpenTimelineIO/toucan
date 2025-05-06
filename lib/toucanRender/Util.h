// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <opentimelineio/anyVector.h>

#include <Imath/ImathBox.h>
#include <Imath/ImathVec.h>

#include <filesystem>
#include <string>

namespace toucan
{
    //! Convert to lower case.
    std::string toLower(const std::string&);

    //! Split the URL protocol.
    std::pair<std::string, std::string> splitURLProtocol(const std::string&);

    //! Get an image sequence file name.
    std::string getSequenceFrame(
        const std::filesystem::path&,
        const std::string& namePrefix,
        int frame,
        int padding,
        const std::string& nameSuffix);

    //! Split the number from a file name.
    std::pair<std::string, std::string> splitFileNameNumber(const std::string&);

    //! Get the zero padding for the given number.
    size_t getNumberPadding(const std::string&);

    //! Find an image sequence given a single frame.
    std::vector<std::filesystem::path> getSequence(const std::filesystem::path&);

    //! Resize and position B to fit into A.
    IMATH_NAMESPACE::Box2i fit(
        const IMATH_NAMESPACE::V2i& a,
        const IMATH_NAMESPACE::V2i& b);

    //! Conversion to any vector.
    OTIO_NS::AnyVector vecToAny(const IMATH_NAMESPACE::V2i&);
    OTIO_NS::AnyVector vecToAny(const IMATH_NAMESPACE::V4f&);
    
    //! Conversion from any vector.
    void anyToVec(const OTIO_NS::AnyVector&, IMATH_NAMESPACE::V2i&);
    void anyToVec(const OTIO_NS::AnyVector&, IMATH_NAMESPACE::V4f&);

    //! Get standard OpenFX plugin search paths.
    //! Includes executable directory, standard OS-specific plugin directories,
    //! and paths from the OFX_PLUGIN_PATH environment variable.
    std::vector<std::filesystem::path> getOpenFXPluginPaths(
        const std::filesystem::path& executablePath);
}
