/*
 * Copyright 2017 deepstreamHub GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DEEPSTREAM_MESSAGE_BUILDER_HPP
#define DEEPSTREAM_MESSAGE_BUILDER_HPP

#include <string>
#include <vector>

#include "message.hpp"

namespace deepstream {
/**
 * This class aids with the construction of deepstream messages.
 */
struct MessageBuilder : public Message {
    typedef Buffer Argument;
    typedef std::vector<Argument> ArgumentList;

    explicit MessageBuilder(const Message::Header&);

    explicit MessageBuilder(Topic topic, Action action, bool is_ack = false);

    void add_argument(const Argument& arg);

    void add_argument(const std::string&);

    virtual std::size_t size_impl_() const;

    virtual const Header& header_impl_() const;

    virtual std::size_t num_arguments_impl_() const;

    virtual Buffer get_impl_(std::size_t) const;

    virtual Buffer to_binary_impl_() const;

    const Message::Header header_;
    ArgumentList arguments_;
};
}

#endif
