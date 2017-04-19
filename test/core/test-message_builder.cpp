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
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <climits>
#include <cstring>

#include <algorithm>
#include <stdexcept>

#include <deepstream/core/buffer.hpp>
#include "src/core/message_builder.hpp"

namespace deepstream {

BOOST_AUTO_TEST_CASE(simple)
{
    const char INPUT[] = "A|REQ|user|password+";
    Buffer output = Message::from_human_readable(INPUT);

    Message::Header header(Topic::AUTH, Action::REQUEST);
    MessageBuilder dummy(header.topic(), header.action(), header.is_ack());
    MessageBuilder builder(header);

    BOOST_CHECK_EQUAL(builder.header(), dummy.header());

    Buffer bin_h = header.to_binary();
    Buffer bin_b = builder.to_binary();

    BOOST_CHECK(std::equal(bin_h.cbegin(), bin_h.cend(), bin_b.cbegin()));

    builder.add_argument(Buffer("user"));
    builder.add_argument(Buffer("password"));

    Buffer binary = builder.to_binary();
    BOOST_REQUIRE_EQUAL(binary.size(), output.size());
    BOOST_CHECK(std::equal(output.cbegin(), output.cend(), binary.begin()));
}

BOOST_AUTO_TEST_CASE(message_with_payload)
{
    const char MSG[] = "A|REQ|user|password+";
    const char USER[] = "user";
    const char PASSWORD[] = "password";

    Message::Header header(Topic::AUTH, Action::REQUEST);
    MessageBuilder::Argument user(USER, USER + std::strlen(USER));
    MessageBuilder::Argument pwd(PASSWORD, PASSWORD + std::strlen(PASSWORD));

    MessageBuilder builder(header);
    builder.add_argument(user);
    builder.add_argument(pwd);

    auto out = builder.to_binary();
    auto bin = Message::from_human_readable(MSG);

    BOOST_REQUIRE_EQUAL(out.size(), bin.size());
    BOOST_CHECK(std::equal(bin.cbegin(), bin.cend(), out.cbegin()));
}

BOOST_AUTO_TEST_CASE(binary_payload)
{
    typedef MessageBuilder::Argument Argument;

    Message::Header header(Topic::EVENT, Action::SUBSCRIBE, true);
    Buffer message = header.to_binary();
    MessageBuilder builder(header);

    const int n = 3;
    for (int u = 0; u < n; ++u) {
        Argument arg;
        message.push_back(ASCII_UNIT_SEPARATOR);

        for (int c = u * CHAR_MAX / n; c < (u + 1) * CHAR_MAX / n; ++c) {
            if (c == ASCII_UNIT_SEPARATOR || c == ASCII_RECORD_SEPARATOR)
                continue;

            arg.push_back(c);
            message.push_back(c);
        }

        builder.add_argument(arg);
    }

    message.push_back(ASCII_RECORD_SEPARATOR);

    auto out = builder.to_binary();

    BOOST_REQUIRE_EQUAL(out.size(), message.size());
    BOOST_CHECK(std::equal(out.cbegin(), out.cend(), message.cbegin()));
}

BOOST_AUTO_TEST_CASE(check_args)
{
    Message::Header header(Topic::EVENT, Action::SUBSCRIBE);
    MessageBuilder builder(header);
    MessageBuilder::Argument arg{ ASCII_UNIT_SEPARATOR };

    BOOST_CHECK_THROW(builder.add_argument(arg), std::invalid_argument);
}
}
