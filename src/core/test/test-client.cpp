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

#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>
#include "../message_builder.hpp"
#include "../state.hpp"

namespace deepstream {

    BOOST_AUTO_TEST_CASE(lifetime)
    {
        auto make_msg = [](Topic topic, Action action) {
            return MessageBuilder(topic, action);
        };
        auto make_ack_msg = [](Topic topic, Action action) {
            return MessageBuilder(topic, action, true);
        };

        State s0 = State::AWAIT_CONNECTION;

        auto msg0 = make_msg(Topic::CONNECTION, Action::CHALLENGE);
        State s1 = transition(s0, msg0, Sender::SERVER);
        BOOST_CHECK_EQUAL(s1, State::CHALLENGING);

        auto msg1 = make_msg(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
        msg1.add_argument(Buffer("URL"));
        State s2 = transition(s1, msg1, Sender::CLIENT);
        BOOST_CHECK_EQUAL(s2, State::CHALLENGING_WAIT);

        auto msg2 = make_ack_msg(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
        State s3 = transition(s2, msg2, Sender::SERVER);
        BOOST_CHECK_EQUAL(s3, State::AWAIT_AUTHENTICATION);

        auto msg3 = make_msg(Topic::AUTH, Action::REQUEST);
        msg3.add_argument(Buffer("{\"username\":\"u\",\"password\":\"p\"}"));
        State s4 = transition(s3, msg3, Sender::CLIENT);
        BOOST_CHECK_EQUAL(s4, State::AUTHENTICATING);

        auto msg4 = make_ack_msg(Topic::AUTH, Action::REQUEST);
        State s5 = transition(s4, msg4, Sender::SERVER);
        BOOST_CHECK_EQUAL(s5, State::CONNECTED);
    }
}
