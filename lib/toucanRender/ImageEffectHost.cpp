// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ImageEffectHost.h"

#include "ImageEffect.h"

#include <ftk/Core/LogSystem.h>

#include <cstdarg>
#include <cstring>
#include <iostream>
#include <sstream>

namespace toucan
{
    namespace
    {
        const std::string logPrefix = "toucan::ImageEffectHost";
    }

    ImageEffectHost::ImageEffectHost(
        const std::shared_ptr<ftk::Context>& context,
        const std::vector<std::filesystem::path>& searchPath) :
        _context(context)
    {
        _propSet.setPointer("host", 0, this);

        _host.host = (OfxPropertySetHandle)&_propSet;
        _host.fetchSuite = &_fetchSuite;

        _suiteInit();
        _pluginInit(searchPath);
    }

    ImageEffectHost::~ImageEffectHost()
    {
        for (const auto& plugin : _plugins)
        {
            OfxStatus ofxStatus = plugin.ofxPlugin->mainEntry(
                kOfxActionUnload,
                nullptr,
                nullptr,
                nullptr);
        }
    }

    std::shared_ptr<IImageNode> ImageEffectHost::createNode(
        const OTIO_NS::AnyDictionary& metaData,
        const std::string& name,
        const std::vector<std::shared_ptr<IImageNode> >& inputs)
    {
        std::shared_ptr<IImageNode> out;
        for (auto& plugin : _plugins)
        {
            if (name == plugin.ofxPlugin->pluginIdentifier)
            {
                out = std::make_shared<ImageEffectNode>(plugin, metaData, name, inputs);
                break;
            }
        }
        return out;
    }

    void ImageEffectHost::_suiteInit()
    {
        _propertySuite.propSetPointer = &PropertySet::setPointer;
        _propertySuite.propSetString = &PropertySet::setString;
        _propertySuite.propSetDouble = &PropertySet::setDouble;
        _propertySuite.propSetInt = &PropertySet::setInt;
        _propertySuite.propSetPointerN = &PropertySet::setPointerN;
        _propertySuite.propSetStringN = &PropertySet::setStringN;
        _propertySuite.propSetDoubleN = &PropertySet::setDoubleN;
        _propertySuite.propSetIntN = &PropertySet::setIntN;
        _propertySuite.propGetPointer = &PropertySet::getPointer;
        _propertySuite.propGetString = &PropertySet::getString;
        _propertySuite.propGetDouble = &PropertySet::getDouble;
        _propertySuite.propGetInt = &PropertySet::getInt;
        _propertySuite.propGetPointerN = &PropertySet::getPointerN;
        _propertySuite.propGetStringN = &PropertySet::getStringN;
        _propertySuite.propGetDoubleN = &PropertySet::getDoubleN;
        _propertySuite.propGetIntN = &PropertySet::getIntN;
        _propertySuite.propReset = &PropertySet::reset;
        _propertySuite.propGetDimension = &PropertySet::getDimension;

        //! \todo Fill out the remaining functions.
        _parameterSuite.paramDefine = &_paramDefine;
        _parameterSuite.paramGetHandle = &_paramGetHandle;
        _parameterSuite.paramGetValue = &_paramGetValue;

        //! \todo Fill out the remaining functions.
        _effectSuite.getPropertySet = &_getPropertySet;
        _effectSuite.getParamSet = &_getParamSet;
        _effectSuite.clipDefine = &_clipDefine;
        _effectSuite.clipGetHandle = &_clipGetHandle;
        _effectSuite.clipGetImage = &_clipGetImage;
        _effectSuite.clipReleaseImage = &_clipReleaseImage;
    }

    void ImageEffectHost::_pluginInit(const std::vector<std::filesystem::path>& searchPath)
    {
        // Find the plugins.
        auto logSystem = _context.lock()->getSystem<ftk::LogSystem>();
        logSystem->print(logPrefix, "Searching for plugins...");
        std::vector<std::filesystem::path> pluginPaths;
        for (const auto& path : searchPath)
        {
            {
                std::stringstream ss;
                ss << "  Search path: " << path.string();
                logSystem->print(logPrefix, ss.str());
            }
            findPlugins(path, pluginPaths);
        }
        if (pluginPaths.empty())
        {
            logSystem->print(logPrefix, "  No plugins found");
        }

        // Load the plugins.
        if (!pluginPaths.empty())
        {
            logSystem->print(logPrefix, "Loading plugins...");
        }
        for (const auto& path : pluginPaths)
        {
            {
                std::stringstream ss;
                ss << "  Path: " << path.string();
                logSystem->print(logPrefix, ss.str());
            }
            try
            {
                auto plugin = std::make_shared<Plugin>(path);
                const int pluginCount = plugin->getCount();
                for (int i = 0; i < pluginCount; ++i)
                {
                    OfxPlugin* ofxPlugin = plugin->getPlugin(i);
                    if (strcmp(ofxPlugin->pluginApi, kOfxImageEffectPluginApi) == 0)
                    {
                        ofxPlugin->setHost(&_host);
                        OfxStatus ofxStatus = ofxPlugin->mainEntry(
                            kOfxActionLoad,
                            nullptr,
                            nullptr,
                            nullptr);
                        switch (ofxStatus)
                        {
                        case kOfxStatOK:
                        case kOfxStatReplyDefault:
                            _plugins.push_back({ plugin, ofxPlugin });
                            break;
                        case kOfxStatErrFatal:
                        {
                            std::stringstream ss;
                            ss << "Fatal error in plugin: " << path.string();
                            throw std::runtime_error(ss.str());
                            break;
                        }
                        case kOfxStatFailed:
                        default:
                            break;
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cout << "ERROR: " << e.what() << std::endl;
            }
        }

        // Initialize the plugins.
        if (!_plugins.empty())
        {
            logSystem->print(logPrefix, "Initializing plugins...");
        }
        for (auto& plugin : _plugins)
        {
            {
                std::stringstream ss;
                ss << "  Plugin: " << plugin.ofxPlugin->pluginIdentifier;
                logSystem->print(logPrefix, ss.str());
            }
            ImageEffectHandle handle = { &plugin };
            OfxStatus ofxStatus = plugin.ofxPlugin->mainEntry(
                kOfxActionDescribe,
                &handle,
                nullptr,
                nullptr);
            int contextCount = 0;
            plugin.propSet.getDimension(kOfxImageEffectPropSupportedContexts, &contextCount);
            for (int i = 0; i < contextCount; ++i)
            {
                char* context = nullptr;
                plugin.propSet.getString(kOfxImageEffectPropSupportedContexts, i, &context);
                if (context)
                {
                    PropertySet propSet;
                    propSet.setString(kOfxImageEffectPropContext, 0, context);
                    {
                        std::stringstream ss;
                        ss << "    Context: " << context;
                        logSystem->print(logPrefix, ss.str());
                    }
                    ofxStatus = plugin.ofxPlugin->mainEntry(
                        kOfxImageEffectActionDescribeInContext,
                        &handle,
                        (OfxPropertySetHandle)&propSet,
                        nullptr);
                }
            }
            for (const auto& param : plugin.paramTypes)
            {
                std::stringstream ss;
                ss << "    \"" << param.first << "\": " << param.second;
                logSystem->print(logPrefix, ss.str());
            }
        }
    }

    const void* ImageEffectHost::_fetchSuite(OfxPropertySetHandle handle, const char* suiteName, int suiteVersion)
    {
        const void* out = nullptr;
        
        PropertySet* hostPropSet = (PropertySet*)handle;
        ImageEffectHost* host = nullptr;
        hostPropSet->getPointer("host", 0, (void**)&host);

        if (strcmp(suiteName, kOfxPropertySuite) == 0)
        {
            out = &host->_propertySuite;
        }
        else if (strcmp(suiteName, kOfxParameterSuite) == 0)
        {
            out = &host->_parameterSuite;
        }
        else if (strcmp(suiteName, kOfxImageEffectSuite) == 0)
        {
            out = &host->_effectSuite;
        }
        return out;
    }
    
    OfxStatus ImageEffectHost::_getPropertySet(OfxImageEffectHandle effectHandle, OfxPropertySetHandle* propHandle)
    {
        ImageEffectHandle* handle = (ImageEffectHandle*)effectHandle;
        *propHandle = (OfxPropertySetHandle)&handle->plugin->propSet;
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_getParamSet(OfxImageEffectHandle effectHandle, OfxParamSetHandle* paramHandle)
    {
        *paramHandle = (OfxParamSetHandle)effectHandle;
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_paramDefine(OfxParamSetHandle effectHandle, const char* paramType, const char* name, OfxPropertySetHandle* propHandle)
    {
        ImageEffectHandle* handle = (ImageEffectHandle*)effectHandle;
        handle->plugin->paramTypes[name] = paramType;
        *propHandle = (OfxPropertySetHandle)&handle->plugin->paramDefs[name];
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_paramGetHandle(OfxParamSetHandle effectHandle, const char* name, OfxParamHandle* paramHandle, OfxPropertySetHandle* propHandle)
    {
        ImageEffectHandle* handle = (ImageEffectHandle*)effectHandle;
        auto i = handle->instance->params.find(name);
        if (i != handle->instance->params.end())
        {
            *paramHandle = (OfxParamHandle)&handle->instance->params[name];
        }
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_paramGetValue(OfxParamHandle handle, ...)
    {
        va_list args;
        va_start(args, handle);
        std::any* a = (std::any*)handle;
        if (a->type() == typeid(bool))
        {
            const bool value = std::any_cast<bool>(*a);
            *va_arg(args, bool*) = value;
        }
        else if (a->type() == typeid(int64_t))
        {
            const int value = std::any_cast<int64_t>(*a);
            *va_arg(args, int64_t*) = value;
        }
        else if (a->type() == typeid(double))
        {
            const double value = std::any_cast<double>(*a);
            *va_arg(args, double*) = value;
        }
        else if (a->type() == typeid(std::string))
        {
            const std::string value = std::any_cast<std::string>(*a);
            *va_arg(args, std::string*) = value;
        }
        else if (a->type() == typeid(OTIO_NS::AnyVector))
        {
            const OTIO_NS::AnyVector value = std::any_cast<OTIO_NS::AnyVector>(*a);
            if (!value.empty() && value[0].type() == typeid(double))
            {
                for (size_t i = 0; i < value.size(); ++i)
                {
                    *va_arg(args, double*) = std::any_cast<double>(value[i]);
                }
            }
            else if (!value.empty() && value[0].type() == typeid(int64_t))
            {
                for (size_t i = 0; i < value.size(); ++i)
                {
                    *va_arg(args, int64_t*) = std::any_cast<int64_t>(value[i]);
                }
            }
        }
        va_end(args);
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_clipDefine(OfxImageEffectHandle effectHandle, const char* name, OfxPropertySetHandle* propHandle)
    {
        ImageEffectHandle* handle = (ImageEffectHandle*)effectHandle;
        *propHandle = (OfxPropertySetHandle)&handle->plugin->clipPropSets[name];
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_clipGetHandle(OfxImageEffectHandle effectHandle, const char* name, OfxImageClipHandle* clip, OfxPropertySetHandle* propHandle)
    {
        ImageEffectHandle* handle = (ImageEffectHandle*)effectHandle;
        *clip = (OfxImageClipHandle)&handle->instance->images[name];
        if (propHandle)
        {
            *propHandle = (OfxPropertySetHandle)&handle->plugin->clipPropSets[name];
        }
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_clipGetImage(OfxImageClipHandle handle, OfxTime, const OfxRectD*, OfxPropertySetHandle* propHandle)
    {
        *propHandle = (OfxPropertySetHandle)handle;
        return kOfxStatOK;
    }

    OfxStatus ImageEffectHost::_clipReleaseImage(OfxPropertySetHandle handle)
    {
        return kOfxStatOK;
    }
}
