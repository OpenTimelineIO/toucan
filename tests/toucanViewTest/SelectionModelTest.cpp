// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FilesModelTest.h"

#include <toucanView/SelectionModel.h>

#include <opentimelineio/clip.h>

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
                model = std::make_shared<SelectionModel>();

                selectionObserver = ftk::ListObserver<SelectionItem>::create(
                    model->observeSelection(),
                    [this](const std::vector<SelectionItem>& value)
                    {
                        selection = value;
                    });
            }

            std::shared_ptr<SelectionModel> model;
            std::vector<SelectionItem> selection;

            std::shared_ptr<ftk::ListObserver<SelectionItem> > selectionObserver;
        };
    }

    void selectionModelTest(
        const std::shared_ptr<ftk::Context>& context,
        const std::filesystem::path& path)
    {
        std::cout << "selectionModelTest" << std::endl;
        {
            const std::filesystem::path timelinePath = path / "Gap.otio";
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> timeline(
                dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(timelinePath.string())));
            const auto clips = timeline->find_clips();

            Test test(context);
            test.model->selectAll(timeline, SelectionType::Clips);
            assert(clips.size() == test.selection.size());

            test.model->clearSelection();
            assert(test.selection.empty());

            test.model->invertSelection(timeline);
            assert(!test.selection.empty());
        }
    }
}
