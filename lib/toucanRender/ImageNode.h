// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <opentimelineio/effect.h>

#include <OpenImageIO/imagebuf.h>

#include <memory>
#include <vector>

namespace toucan
{
    class ImageEffectHost;

    //! Base class for image nodes.
    class IImageNode : public std::enable_shared_from_this<IImageNode>
    {
    public:
        IImageNode(
            const std::string& name,
            const std::vector<std::shared_ptr<IImageNode> >& = {});

        virtual ~IImageNode() = 0;

        //! Get the name.
        const std::string& getName() const;

        //! Get the label.
        virtual std::string getLabel() const;

        //! Get the inputs.
        const std::vector<std::shared_ptr<IImageNode> >& getInputs() const;

        //! Set the inputs.
        void setInputs(const std::vector<std::shared_ptr<IImageNode> >&);

        //! Set the time.
        void setTime(const OTIO_NS::RationalTime&);

        //! Execute the image operation.
        virtual OIIO::ImageBuf exec() = 0;

        //! Generate a Grapviz graph
        std::vector<std::string> graph(const std::string& name);

    protected:
        void _graph(
            const std::shared_ptr<IImageNode>&,
            std::vector<std::string>&);
        std::string _getGraphName() const;

        std::string _name;
        std::vector<std::shared_ptr<IImageNode> > _inputs;
        OTIO_NS::RationalTime _time;
    };
}
