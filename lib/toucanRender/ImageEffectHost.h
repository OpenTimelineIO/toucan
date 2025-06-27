// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageEffect.h>

#include <feather-tk/core/Context.h>

#include <OpenImageIO/imagebuf.h>

#include <filesystem>

namespace toucan
{
    //! Image effect host.
    class ImageEffectHost : public std::enable_shared_from_this<ImageEffectHost>
    {
    public:
        ImageEffectHost(
            const std::shared_ptr<feather_tk::Context>&,
            const std::vector<std::filesystem::path>& searchPath);

        ~ImageEffectHost();

        //! Create an image node.
        std::shared_ptr<IImageNode> createNode(
            const OTIO_NS::AnyDictionary&,
            const std::string& name,
            const std::vector<std::shared_ptr<IImageNode> >& = {});

    private:
        void _suiteInit();
        void _pluginInit(const std::vector<std::filesystem::path>& searchPath);

        static const void* _fetchSuite(OfxPropertySetHandle, const char* suiteName, int suiteVersion);
        static OfxStatus _getPropertySet(OfxImageEffectHandle, OfxPropertySetHandle*);
        static OfxStatus _getParamSet(OfxImageEffectHandle, OfxParamSetHandle*);
        static OfxStatus _paramDefine(OfxParamSetHandle, const char* paramType, const char* name, OfxPropertySetHandle*);
        static OfxStatus _paramGetHandle(OfxParamSetHandle, const char* name, OfxParamHandle*, OfxPropertySetHandle*);
        static OfxStatus _paramGetValue(OfxParamHandle, ...);
        static OfxStatus _clipDefine(OfxImageEffectHandle, const char* name, OfxPropertySetHandle*);
        static OfxStatus _clipGetHandle(OfxImageEffectHandle, const char* name, OfxImageClipHandle*, OfxPropertySetHandle*);
        static OfxStatus _clipGetImage(OfxImageClipHandle, OfxTime, const OfxRectD*, OfxPropertySetHandle*);
        static OfxStatus _clipReleaseImage(OfxPropertySetHandle);

        std::weak_ptr<feather_tk::Context> _context;
        PropertySet _propSet;
        OfxHost _host;
        OfxPropertySuiteV1 _propertySuite;
        OfxParameterSuiteV1 _parameterSuite;
        OfxImageEffectSuiteV1 _effectSuite;
        std::vector<ImageEffectPlugin> _plugins;
    };
}
