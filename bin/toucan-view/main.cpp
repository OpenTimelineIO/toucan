// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "toucanView/App.h"

#include <feather-tk/core/Context.h>

#include <iostream>

FEATHER_TK_MAIN()
{
    try
    {
        auto context = feather_tk::Context::create();
        auto args = feather_tk::convert(argc, argv);
        auto app = toucan::App::create(context, args);
        if (app->getExit() != 0)
            return app->getExit();
        app->run();
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
