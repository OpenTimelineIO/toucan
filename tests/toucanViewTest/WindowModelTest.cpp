// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "WindowModelTest.h"

#include <toucanView/WindowModel.h>

#include <cassert>
#include <iostream>

namespace toucan
{
    namespace
    {
        class Test
        {
        public:
            Test(const std::shared_ptr<ftk::Context>& context)
            {
                model = std::make_shared<WindowModel>(context, nullptr);

                componentsObserver = ftk::MapObserver<WindowComponent, bool>::create(
                    model->observeComponents(),
                    [this](const std::map<WindowComponent, bool>& value)
                    {
                        components = value;
                    });

                tooltipsObserver = ftk::ValueObserver<bool>::create(
                    model->observeTooltips(),
                    [this](bool value)
                    {
                        tooltips = value;
                    });
            }

            std::shared_ptr<WindowModel> model;
            std::map<WindowComponent, bool> components;
            bool tooltips = false;

            std::shared_ptr<ftk::MapObserver<WindowComponent, bool> > componentsObserver;
            std::shared_ptr<ftk::ValueObserver<bool> > tooltipsObserver;
        };
    }

    void windowModelTest(const std::shared_ptr<ftk::Context>& context)
    {
        std::cout << "windowModelTest" << std::endl;
        {
            Test test(context);
            std::map<WindowComponent, bool> components =
            {
                { WindowComponent::ToolBar, true },
                { WindowComponent::Playback, true }
            };
            test.model->setComponents(components);
            assert(components == test.components);
            test.model->setComponent(WindowComponent::ToolBar, false);
            assert(!test.components[WindowComponent::ToolBar]);
        }
    }
}
