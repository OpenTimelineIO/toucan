// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ImageEffect.h"

#include <toucanRender/Util.h>

namespace toucan
{
    ImageEffectNode::ImageEffectNode(
        ImageEffectPlugin& plugin,
        const OTIO_NS::AnyDictionary& metaData,
        const std::string& name,
        const std::vector<std::shared_ptr<IImageNode> >& inputs) :
        IImageNode(name, inputs),
        _plugin(plugin),
        _instance(new ImageEffectInstance),
        _handle{ &plugin, _instance.get() },
        _metaData(metaData)
    {
        // Set default values.
        for (const auto& param : _plugin.paramDefs)
        {
            auto props = param.second.getStringProperties();
            auto i = std::find(props.begin(), props.end(), kOfxParamPropDefault);
            if (i != props.end())
            {
                char* s = nullptr;
                param.second.getString(kOfxParamPropDefault, 0, &s);
                if (s)
                {
                    _instance->params[param.first] = std::string(s);
                }
            }
            props = param.second.getDoubleProperties();
            i = std::find(props.begin(), props.end(), kOfxParamPropDefault);
            if (i != props.end())
            {
                double d = 0.0;
                param.second.getDouble(kOfxParamPropDefault, 0, &d);
                _instance->params[param.first] = d;
            }
            props = param.second.getIntProperties();
            i = std::find(props.begin(), props.end(), kOfxParamPropDefault);
            if (i != props.end())
            {
                int i = 0;
                param.second.getInt(kOfxParamPropDefault, 0, &i);
                _instance->params[param.first] = i;
            }
        }

        // Set values.
        for (const auto& i : metaData)
        {
            _instance->params[i.first] = i.second;
        }

        // Create the plugin instance.
        OfxStatus ofxStatus = _plugin.ofxPlugin->mainEntry(
            kOfxActionCreateInstance,
            &_handle,
            nullptr,
            nullptr);
    }

    ImageEffectNode::~ImageEffectNode()
    {
        // Destroy the plugin instance.
        OfxStatus ofxStatus = _plugin.ofxPlugin->mainEntry(
            kOfxActionDestroyInstance,
            &_handle,
            nullptr,
            nullptr);
    }

    OIIO::ImageBuf ImageEffectNode::exec()
    {
        OIIO::ImageBuf out;

        // Initialize the images.
        std::vector<OIIO::ImageBuf> inputs;
        IMATH_NAMESPACE::V2i size = IMATH_NAMESPACE::V2i(0, 0);
        auto i = _metaData.find("size");
        if (i != _metaData.end() && i->second.has_value())
        {
            anyToVec(std::any_cast<OTIO_NS::AnyVector>(i->second), size);
        }
        char* context = nullptr;
        _plugin.propSet.getString(kOfxImageEffectPropSupportedContexts, 0, &context);
        if (strcmp(context, kOfxImageEffectContextGenerator) == 0)
        {
            out = OIIO::ImageBuf(OIIO::ImageSpec(size.x, size.y, 4));
            _instance->images["Output"] = bufToPropSet(out);
        }
        else if (
            strcmp(context, kOfxImageEffectContextFilter) == 0 &&
            !_inputs.empty() &&
            _inputs[0])
        {
            inputs.push_back(_inputs[0]->exec());
            auto spec = inputs[0].spec();
            if (size.x > 0 && size.y > 0)
            {
                spec.width = size.x;
                spec.height = size.y;
            }
            out = OIIO::ImageBuf(spec);
            _instance->images["Source"] = bufToPropSet(inputs[0]);
            _instance->images["Output"] = bufToPropSet(out);
        }
        else if (
            strcmp(context, kOfxImageEffectContextTransition) == 0 &&
            _inputs.size() > 1 &&
            _inputs[0] &&
            _inputs[1])
        {
            inputs.push_back(_inputs[0]->exec());
            inputs.push_back(_inputs[1]->exec());
            auto spec = inputs[0].spec();
            if (size.x > 0 && size.y > 0)
            {
                spec.width = size.x;
                spec.height = size.y;
            }
            out = OIIO::ImageBuf(spec);
            _instance->images["SourceFrom"] = bufToPropSet(inputs[0]);
            _instance->images["SourceTo"] = bufToPropSet(inputs[1]);
            _instance->images["Output"] = bufToPropSet(out);
        }

        // Render.
        const auto& spec = out.spec();
        if (spec.width > 0 && spec.height > 0)
        {
            PropertySet args;
            args.setDouble(kOfxPropTime, 0, _time.value());
            OfxRectI bounds;
            bounds.x1 = 0;
            bounds.x2 = spec.width;
            bounds.y1 = 0;
            bounds.y2 = spec.height;
            args.setIntN(kOfxImageEffectPropRenderWindow, 4, &bounds.x1);

            _plugin.ofxPlugin->mainEntry(
                kOfxImageEffectActionRender,
                &_handle,
                (OfxPropertySetHandle)&args,
                nullptr);
        }

        return out;
    }
}
