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

#include <deepstream/buffer.hpp>
#include <deepstream/message_builder.hpp>
#include "../src/random.hpp"
#include "../src/use.hpp"

namespace deepstream {
namespace random {

    BOOST_AUTO_TEST_CASE(simple)
    {
        Engine::result_type seed = 1;
        Engine engine(seed);

        const Message::Header& h = make_header(&engine);
        use(h);

        std::size_t arg_size = 10;
        Buffer argument = make_argument(&engine, arg_size, arg_size);
        BOOST_CHECK_EQUAL(argument.size(), arg_size);

        MessageBuilder builder = make_message(&engine);
        use(builder);
    }
}
}
