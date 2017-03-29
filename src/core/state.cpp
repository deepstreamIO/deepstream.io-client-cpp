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

#include "message.hpp"
#include "use.hpp"
#include <deepstream/core/client.hpp>
#include <ostream>

namespace deepstream {

std::ostream& operator<<(std::ostream& os, ConnectionState state)
{
    os << static_cast<int>(state);
    return os;
}

ConnectionState transition(ConnectionState state, const Message& message, Sender sender)
{
    assert(state != ConnectionState::ERROR);
    assert(state != ConnectionState::DISCONNECTED);

    const Topic topic = message.topic();
    const Action action = message.action();
    const bool is_ack = message.is_ack();

    const auto expected_num_args = Message::num_arguments(message.header());
    const std::size_t& num_args = message.num_arguments();

    use(expected_num_args);
    use(num_args);
    assert(num_args >= expected_num_args.first);
    assert(num_args <= expected_num_args.second);

    // ping/pong
    // heartbeats are implemented within deepstream and independent of
    // websocket heartbeats (https://tools.ietf.org/html/rfc6455#section-5.5)
    if (topic == Topic::CONNECTION && action == Action::PING && sender == Sender::SERVER) {
        assert(!is_ack);

        return state;
    }

    if (topic == Topic::CONNECTION && action == Action::PONG && sender == Sender::CLIENT) {
        assert(!is_ack);

        return state;
    }

    // actual state transitions
    if (state == ConnectionState::AWAIT_CONNECTION && topic == Topic::CONNECTION && action == Action::CHALLENGE && sender == Sender::SERVER) {
        assert(!is_ack);

        return ConnectionState::CHALLENGING;
    }

    if (state == ConnectionState::CHALLENGING && topic == Topic::CONNECTION && action == Action::CHALLENGE_RESPONSE && !is_ack && sender == Sender::CLIENT) {
        return ConnectionState::CHALLENGING_WAIT;
    }

    if (state == ConnectionState::CHALLENGING_WAIT && topic == Topic::CONNECTION && action == Action::CHALLENGE_RESPONSE && is_ack && sender == Sender::SERVER) {
        return ConnectionState::AWAIT_AUTHENTICATION;
    }

    if (state == ConnectionState::CHALLENGING_WAIT && topic == Topic::CONNECTION && action == Action::REDIRECT && sender == Sender::SERVER) {
        assert(!is_ack);

        return ConnectionState::AWAIT_CONNECTION;
    }

    if (state == ConnectionState::CHALLENGING_WAIT && topic == Topic::CONNECTION && action == Action::REJECT && sender == Sender::SERVER) {
        assert(!is_ack);

        return ConnectionState::DISCONNECTED;
    }

    if (state == ConnectionState::AWAIT_AUTHENTICATION && topic == Topic::AUTH && action == Action::REQUEST && !is_ack && sender == Sender::CLIENT) {
        return ConnectionState::AUTHENTICATING;
    }

    if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::REQUEST && is_ack && sender == Sender::SERVER) {
        return ConnectionState::CONNECTED;
    }

    if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS && sender == Sender::SERVER) {
        assert(!is_ack);

        return ConnectionState::DISCONNECTED;
    }

    if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_INVALID_AUTH_DATA && sender == Sender::SERVER) {
        assert(!is_ack);

        return ConnectionState::AWAIT_AUTHENTICATION;
    }

    if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_INVALID_AUTH_MSG && sender == Sender::SERVER) {
        assert(!is_ack);

        return ConnectionState::DISCONNECTED;
    }

    if (state == ConnectionState::CONNECTED)
        return state;

    return ConnectionState::ERROR;
}
}
