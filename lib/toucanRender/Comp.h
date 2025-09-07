// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <toucanRender/ImageNode.h>

namespace toucan
{
    //! Compositing node.
    class CompNode : public IImageNode
    {
    public:
        CompNode(const std::vector<std::shared_ptr<IImageNode> >& = {});

        virtual ~CompNode();

        //! Set whether images are pre-multiplied before compositing.
        void setPremult(bool);

        //! Set whether images are resized before compositing.
        void setResize(bool);

        OIIO::ImageBuf exec(const OTIO_NS::RationalTime&) override;

    private:
        bool _premult = false;
        bool _resize = true;
    };
}
