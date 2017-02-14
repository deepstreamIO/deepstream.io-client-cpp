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
#ifndef DEEPSTREAM_RANDOM_HPP
#define DEEPSTREAM_RANDOM_HPP

#include <random>

#include <deepstream/message.hpp>

namespace deepstream {
struct Buffer;
struct MessageBuilder;

/**
 * This module generates random deepstream messages or random components of
 * deepstream messages (header, payload).
 */
namespace random {
    typedef std::mt19937_64 Engine;

    /**
 * @return A random deepstream header
 */
    const Message::Header& make_header(Engine*);

    /**
 * @return Random payload within the given size bounds without
 * ASCII record separator and without ASCII unit separator
 */
    Buffer make_argument(Engine*, std::size_t min_size = 1,
        std::size_t max_size = 10);

    /**
 * @return A valid message with random header and random payload
 */
    MessageBuilder make_message(Engine*);

    /**
 * @return A message with a given header and random payload
 */
    MessageBuilder make_message(Engine*, const Message::Header&);
}
}

#endif
