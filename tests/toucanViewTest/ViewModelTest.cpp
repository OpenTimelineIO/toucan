// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ViewModelTest.h"

#include <toucanView/ViewModel.h>

#include <cassert>
#include <iostream>

namespace toucan
{
    namespace
    {
        class Test
        {
        public:
            Test(const std::shared_ptr<feather_tk::Context>& context)
            {
                model = std::make_shared<ViewModel>(context);

                zoomInObserver = feather_tk::ValueObserver<bool>::create(
                    model->observeZoomIn(),
                    [this](bool value)
                    {
                        zoomIn = value;
                    });

                zoomOutObserver = feather_tk::ValueObserver<bool>::create(
                    model->observeZoomOut(),
                    [this](bool value)
                    {
                        zoomOut = value;
                    });

                zoomResetObserver = feather_tk::ValueObserver<bool>::create(
                    model->observeZoomReset(),
                    [this](bool value)
                    {
                        zoomReset = value;
                    });

                frameViewObserver = feather_tk::ValueObserver<bool>::create(
                    model->observeFrameView(),
                    [this](bool value)
                    {
                        frameView = value;
                    });
            }

            std::shared_ptr<ViewModel> model;
            bool zoomIn = false;
            bool zoomOut = false;
            bool zoomReset = false;
            bool frameView = false;

            std::shared_ptr<feather_tk::ValueObserver<bool> > zoomInObserver;
            std::shared_ptr<feather_tk::ValueObserver<bool> > zoomOutObserver;
            std::shared_ptr<feather_tk::ValueObserver<bool> > zoomResetObserver;
            std::shared_ptr<feather_tk::ValueObserver<bool> > frameViewObserver;
        };
    }

    void viewModelTest(const std::shared_ptr<feather_tk::Context>& context)
    {
        std::cout << "viewModelTest" << std::endl;
        {
            Test test(context);
            test.model->zoomIn();
            assert(test.zoomIn);
            test.model->zoomOut();
            assert(test.zoomOut);
            test.model->zoomReset();
            assert(test.zoomReset);
        }
        {
            Test test(context);
            test.model->setFrameView(true);
            assert(test.frameView);
            test.model->setFrameView(false);
            assert(!test.frameView);
        }
    }
}
