// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FilesModelTest.h"

#include <toucanView/FilesModel.h>

#include <cassert>
#include <iostream>

namespace toucan
{
    namespace
    {
        class Test
        {
        public:
            Test(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<ImageEffectHost>& host)
            {
                model = std::make_shared<FilesModel>(context, nullptr, host);

                filesObserver = ftk::ListObserver<std::shared_ptr<File> >::create(
                    model->observeFiles(),
                    [this](const std::vector<std::shared_ptr<File> >& value)
                    {
                        files = value;
                    });

                addObserver = ftk::ValueObserver<int>::create(
                    model->observeAdd(),
                    [this](int value)
                    {
                        add = value;
                    });

                removeObserver = ftk::ValueObserver<int>::create(
                    model->observeRemove(),
                    [this](int value)
                    {
                        remove = value;
                    });

                currentObserver = ftk::ValueObserver<std::shared_ptr<File>>::create(
                    model->observeCurrent(),
                    [this](const std::shared_ptr<File>& value)
                    {
                        current = value;
                    });

                currentIndexObserver = ftk::ValueObserver<int>::create(
                    model->observeCurrentIndex(),
                    [this](int value)
                    {
                        currentIndex = value;
                    });

                bFileObserver = ftk::ValueObserver<std::shared_ptr<File> >::create(
                    model->observeBFile(),
                    [this](const std::shared_ptr<File>& value)
                    {
                        bFile = value;
                    });

                bIndexObserver = ftk::ValueObserver<int>::create(
                    model->observeBIndex(),
                    [this](int value)
                    {
                        bIndex = value;
                    });

                compareOptionsObserver = ftk::ValueObserver<CompareOptions>::create(
                    model->observeCompareOptions(),
                    [this](const CompareOptions& value)
                    {
                        compareOptions = value;
                    });
            }

            std::shared_ptr<FilesModel> model;
            std::vector<std::shared_ptr<File> > files;
            int add = -1;
            int remove = -1;
            std::shared_ptr<File> current;
            int currentIndex = -1;
            std::shared_ptr<File> bFile;
            int bIndex = -1;
            CompareOptions compareOptions;

            std::shared_ptr<ftk::ListObserver<std::shared_ptr<File> > > filesObserver;
            std::shared_ptr<ftk::ValueObserver<int> > addObserver;
            std::shared_ptr<ftk::ValueObserver<int> > removeObserver;
            std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > currentObserver;
            std::shared_ptr<ftk::ValueObserver<int> > currentIndexObserver;
            std::shared_ptr<ftk::ValueObserver<std::shared_ptr<File> > > bFileObserver;
            std::shared_ptr<ftk::ValueObserver<int> > bIndexObserver;
            std::shared_ptr<ftk::ValueObserver<CompareOptions> > compareOptionsObserver;
        };
    }

    void filesModelTest(
        const std::shared_ptr<ftk::Context>& context,
        const std::shared_ptr<ImageEffectHost>& host,
        const std::filesystem::path& path)
    {
        std::cout << "filesModelTest" << std::endl;
        const std::filesystem::path timelinePath = path / "CompositeTracks.otio";
        const std::filesystem::path timelinePath2 = path / "Gap.otio";
        {
            Test test(context, host);
            test.model->open(timelinePath);
            assert(1 == test.files.size());
            assert(0 == test.add);
            assert(0 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[0]);

            test.model->close();
            assert(test.files.empty());
            assert(0 == test.remove);
            assert(-1 == test.currentIndex);
            assert(!test.current);
        }
        {
            Test test(context, host);
            test.model->open(timelinePath);
            test.model->open(timelinePath2);
            assert(2 == test.files.size());
            assert(1 == test.add);
            assert(1 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[1]);

            test.model->close();
            assert(1 == test.files.size());
            assert(1 == test.remove);
            assert(0 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[0]);

            test.model->closeAll();
            assert(test.files.empty());
            assert(0 == test.remove);
            assert(-1 == test.currentIndex);
            assert(!test.current);

            test.model->open(timelinePath);
            test.model->open(timelinePath2);
            test.model->close(0);
            assert(1 == test.files.size());
            assert(0 == test.remove);
            assert(0 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[0]);

            test.model->close(0);
            assert(test.files.empty());
            assert(0 == test.remove);
            assert(-1 == test.currentIndex);
            assert(!test.current);
        }
        {
            Test test(context, host);
            test.model->open(timelinePath);
            test.model->open(timelinePath2);

            test.model->setCurrentIndex(0);
            test.model->setCurrentIndex(0);
            assert(0 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[0]);
            test.model->setCurrentIndex(1);
            assert(1 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[1]);

            test.model->next();
            assert(0 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[0]);

            test.model->next();
            assert(1 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[1]);

            test.model->prev();
            assert(0 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[0]);

            test.model->prev();
            assert(1 == test.currentIndex);
            assert(test.current);
            assert(test.current == test.files[1]);
        }
        {
            Test test(context, host);
            test.model->open(timelinePath);
            test.model->open(timelinePath2);

            test.model->setBIndex(1);
            assert(test.files[1] == test.bFile);
            assert(1 == test.bIndex);

            test.model->close();
            assert(!test.bFile);
            assert(-1 == test.bIndex);

            test.model->setBIndex(0);
            assert(test.files[0] == test.bFile);
            assert(0 == test.bIndex);

            test.model->closeAll();
            assert(!test.bFile);
            assert(-1 == test.bIndex);
        }
        {
            Test test(context, host);
            test.model->open(timelinePath);
            test.model->open(timelinePath2);

            test.model->setBIndex(0);
            test.model->setBIndex(-1);
            assert(!test.bFile);
            assert(-1 == test.bIndex);
        }
    }
}
