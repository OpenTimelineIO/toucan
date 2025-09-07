// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageNode.h>
#include <toucanRender/Plugin.h>
#include <toucanRender/PropertySet.h>

#include <OpenFX/ofxImageEffect.h>

#include <opentimelineio/anyDictionary.h>

namespace toucan
{
    //! Image effect plugin.
    struct ImageEffectPlugin
    {
        std::shared_ptr<Plugin> plugin;
        OfxPlugin* ofxPlugin = nullptr;
        PropertySet propSet;
        std::map<std::string, PropertySet> clipPropSets;
        std::map<std::string, std::string> paramTypes;
        std::map<std::string, PropertySet> paramDefs;
    };

    //! Image effect instance.
    struct ImageEffectInstance
    {
        std::map<std::string, std::any> params;
        std::map<std::string, PropertySet> images;
    };

    //! Image effect handle.
    struct ImageEffectHandle
    {
        ImageEffectPlugin* plugin = nullptr;
        ImageEffectInstance* instance = nullptr;
    };

    //! Image effect node.
    class ImageEffectNode : public IImageNode
    {
    public:
        ImageEffectNode(
            ImageEffectPlugin&,
            const OTIO_NS::AnyDictionary& metaData,
            const std::string& name,
            const std::vector<std::shared_ptr<IImageNode> >& = {});

        virtual ~ImageEffectNode();

        OIIO::ImageBuf exec(const OTIO_NS::RationalTime&) override;

    private:
        ImageEffectPlugin& _plugin;
        std::unique_ptr<ImageEffectInstance> _instance;
        ImageEffectHandle _handle;
        OTIO_NS::AnyDictionary _metaData;
    };
}
