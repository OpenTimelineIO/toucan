// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2024 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>
#include <string>

namespace toucan
{
    //! Log message type.
    enum class MessageLogType
    {
        Info,
        Warning,
        Error
    };

    //! Message logging.
    class MessageLog : public std::enable_shared_from_this<MessageLog>
    {
    public:
        void log(
            const std::string& prefix,
            const std::string& message,
            MessageLogType = MessageLogType::Info);
    };
}