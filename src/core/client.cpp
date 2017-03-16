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

#include <cstdint>

#include <algorithm>
#include <chrono>
#include <stdexcept>

#include <ostream>

#include "client-impl.hpp"
#include "message.hpp"
#include "use.hpp"
#include "websockets.hpp"
#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>
#include <deepstream/core/error_handler.hpp>

#include <cassert>

namespace deepstream {

Client::Client(const std::string& uri, std::unique_ptr<ErrorHandler> p_eh)
    : p_impl_(ClientImpl::make(uri, std::move(p_eh)).release())
    , event([this](const Message& message) -> bool {
        assert(p_impl_);
        return p_impl_->send_(message) == websockets::State::OPEN;
    })
    , presence([this](const Message& message) -> bool {
        assert(p_impl_);
        return p_impl_->send_(message) == websockets::State::OPEN;
    })
{
}

Client::Client(const std::string& uri)
    : Client(uri, std::unique_ptr<ErrorHandler>(new ErrorHandler()))
{
}

Client::~Client() { delete p_impl_; }

bool Client::login() { return p_impl_->login("{}"); }

bool Client::login(const std::string& auth, Buffer* p_user_data)
{
    return p_impl_->login(auth, p_user_data);
}

void Client::close() { return p_impl_->close(); }

client::State Client::getConnectionState()
{
    return p_impl_->getConnectionState();
}

void Client::process_messages()
{
    return p_impl_->process_messages(&event, &presence);
}
}

namespace deepstream {
namespace client {

    std::ostream& operator<<(std::ostream& os, State state)
    {
        os << static_cast<int>(state);
        return os;
    }

    State transition(State state, const Message& message, Sender sender)
    {
        assert(state != State::ERROR);
        assert(state != State::DISCONNECTED);

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
        if (state == State::AWAIT_CONNECTION && topic == Topic::CONNECTION && action == Action::CHALLENGE && sender == Sender::SERVER) {
            assert(!is_ack);

            return State::CHALLENGING;
        }

        if (state == State::CHALLENGING && topic == Topic::CONNECTION && action == Action::CHALLENGE_RESPONSE && !is_ack && sender == Sender::CLIENT) {
            return State::CHALLENGING_WAIT;
        }

        if (state == State::CHALLENGING_WAIT && topic == Topic::CONNECTION && action == Action::CHALLENGE_RESPONSE && is_ack && sender == Sender::SERVER) {
            return State::AWAIT_AUTHENTICATION;
        }

        if (state == State::CHALLENGING_WAIT && topic == Topic::CONNECTION && action == Action::REDIRECT && sender == Sender::SERVER) {
            assert(!is_ack);

            return State::AWAIT_CONNECTION;
        }

        if (state == State::CHALLENGING_WAIT && topic == Topic::CONNECTION && action == Action::REJECT && sender == Sender::SERVER) {
            assert(!is_ack);

            return State::DISCONNECTED;
        }

        if (state == State::AWAIT_AUTHENTICATION && topic == Topic::AUTH && action == Action::REQUEST && !is_ack && sender == Sender::CLIENT) {
            return State::AUTHENTICATING;
        }

        if (state == State::AUTHENTICATING && topic == Topic::AUTH && action == Action::REQUEST && is_ack && sender == Sender::SERVER) {
            return State::CONNECTED;
        }

        if (state == State::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS && sender == Sender::SERVER) {
            assert(!is_ack);

            return State::DISCONNECTED;
        }

        if (state == State::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_INVALID_AUTH_DATA && sender == Sender::SERVER) {
            assert(!is_ack);

            return State::AWAIT_AUTHENTICATION;
        }

        if (state == State::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_INVALID_AUTH_MSG && sender == Sender::SERVER) {
            assert(!is_ack);

            return State::DISCONNECTED;
        }

        if (state == State::CONNECTED)
            return state;

        return State::ERROR;
    }
}
}
