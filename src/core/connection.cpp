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
#include <sstream>

#include "connection.hpp"
#include "message.hpp"
#include "message_builder.hpp"
#include "use.hpp"
#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>
#include <deepstream/core/error_handler.hpp>
#include <deepstream/core/event.hpp>
#include <deepstream/core/presence.hpp>

#include <cassert>

#ifndef NDEBUG
#include <iostream>
#define DEBUG_MSG(str) do { std::cout << "\033[1,31m#\033[0m " << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

namespace deepstream {

    Connection::Connection(const std::string &uri, WSHandler &ws_handler,
            ErrorHandler &error_handler, Event &event, Presence &presence)
        : state_(ConnectionState::CLOSED)
        , error_handler_(error_handler)
        , ws_handler_(ws_handler)
        , p_login_callback_(nullptr)
        , p_auth_params_(nullptr)
        , event_(event)
        , presence_(presence)
        , deliberate_close_(false)
        , reconnection_attempt_(0)
    {
        assert(ws_handler.state() == WSState::CLOSED);

        ws_handler.URI(uri);

        using namespace std::placeholders;
        ws_handler.on_message(std::bind(&Connection::on_message, this, _1));
        ws_handler.on_error(std::bind(&Connection::on_error, this, _1));
        ws_handler.on_open(std::bind(&Connection::on_open, this));
        ws_handler.on_close(std::bind(&Connection::on_close, this));

        ws_handler.open();
    }

    void Connection::login(const Buffer& auth_params, const Client::LoginCallback &callback)
    {
        if (state_ == ConnectionState::OPEN) {
            return;
        }

        p_login_callback_ = std::unique_ptr<Client::LoginCallback>(new Client::LoginCallback(callback));

        p_auth_params_ = std::unique_ptr<Buffer>(new Buffer(auth_params));

        if (state_ == ConnectionState::AWAIT_AUTHENTICATION) {
            send_authentication_request();
        }
    }

    void Connection::send_authentication_request()
    {
        assert(state_ == ConnectionState::AWAIT_AUTHENTICATION);
        assert(p_auth_params_);

        MessageBuilder authentication_request(Topic::AUTH, Action::REQUEST);
        authentication_request.add_argument(*p_auth_params_);

        send(authentication_request);
    }

    void Connection::close()
    {
        deliberate_close_ = true;
        ws_handler_.close();
    }

    ConnectionState Connection::state()
    {
        return state_;
    }

    void Connection::state(const ConnectionState state)
    {
        if (state != state_) {
            state_ = state;
            on_connection_state_change_();
        }
    }

    void Connection::on_connection_state_change_()
    {
        DEBUG_MSG("Connection state change: " << state_);
        event_.on_connection_state_change_(state_);
        //presence_.on_connection_state_change_(state_);
    }

    void Connection::on_message(const Buffer &&raw_message)
    {
        Buffer buffer(raw_message.size() + 2, 0);
        std::copy(raw_message.cbegin(), raw_message.cend(), buffer.begin());

        auto parser_result = parser::execute(buffer.data(), buffer.size());
        const parser::ErrorList& errors = parser_result.second;

        for (auto it = errors.cbegin(); it != errors.cend(); ++it) {
            const parser::Error &error = *it;
            std::stringstream error_message;
            error_message << "parser error: " << error << " \""
                << Message::to_human_readable(raw_message) << "\"";
            error_handler_.on_error(error_message.str());
        }

        parser::MessageList parsed_messages(std::move(parser_result.first));

        for (auto it = parsed_messages.cbegin(); it != parsed_messages.cend(); ++it) {
            const Message &parsed_message = *it;
            DEBUG_MSG("Message received: " << parsed_message.header());

            switch (parsed_message.topic()) {
                case Topic::EVENT:
                    event_.notify_(parsed_message);
                    break;

                case Topic::PRESENCE:
                    presence_.notify_(parsed_message);
                    break;

                case Topic::CONNECTION:
                    handle_connection_response(parsed_message);
                    break;

                case Topic::AUTH:
                    handle_authentication_response(parsed_message);
                    break;

                default:
                    {
                        std::stringstream error_message;
                        error_message << "unsolicited message: " << parsed_message.header();
                        error_handler_.on_error(error_message.str());
                        assert(0);
                    }
            }

        }
    }

    void Connection::handle_connection_response(const Message &message)
    {
        ConnectionState new_state = transition_incoming(state_, message);

        if (new_state == ConnectionState::ERROR) {
            std::stringstream error_message;
            error_message << "connection error state reached, previous state: " << state_
                << ", message header: " << message.header();
            error_handler_.on_error(error_message.str());
        }

        state(new_state);
        switch(message.action()) {
            case Action::PING:
                {
                    const MessageBuilder pong(Topic::CONNECTION, Action::PONG);
                    send(pong);
                } break;
            case Action::CHALLENGE:
                {
                    MessageBuilder challenge_response(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
                    challenge_response.add_argument(ws_handler_.URI());
                    send(challenge_response);
                } break;
            case Action::CHALLENGE_RESPONSE:
                {
                    if (p_auth_params_) {
                        send_authentication_request();
                    }
                } break;
            case Action::REDIRECT:
                {
                    if (message.num_arguments() < 1) {
                        error_handler_.on_error("No URI given in connection redirect message");
                        break;
                    }
                    const Buffer &uri_buff(message[0]);
                    const std::string uri(uri_buff.cbegin(), uri_buff.cend());
                    DEBUG_MSG("redirecting to \"" << uri << "\"");
                    ws_handler_.URI(uri);
                } break;
            default:
                {
                    std::stringstream error_message;
                    error_message << "connection message with unknown action " << state_
                        << std::endl << '\t' << message.header();
                    error_handler_.on_error(error_message.str());
                }
        }
    }

    void Connection::handle_authentication_response(const Message &message)
    {
        switch(message.action()) {
            case Action::ERROR_TOO_MANY_AUTH_ATTEMPTS:
                {
                    error_handler_.on_error("too many authentication attempts");
                    close();
                } break;
            case Action::ERROR_INVALID_AUTH_DATA:
                {
                    error_handler_.on_error("invalid auth data");
                    close();
                } break;
            case Action::ERROR_INVALID_AUTH_MSG:
                {
                    error_handler_.on_error("invalid auth message");
                    close();
                } break;
            case Action::REQUEST:
                {
                    assert(message.is_ack());
                    if (p_login_callback_) {
                        const auto login_callback = *p_login_callback_;
                        if (message.num_arguments() < 1) {
                            Buffer null{ static_cast<char>(PayloadType::NULL_) };
                            login_callback(std::move(null));
                        } else {
                            login_callback(message[0]);
                        }
                    }
                } break;
            default:
                {
                    std::stringstream error_message;
                    error_message << "authentication message with unknown action" << state_
                        << ", message header: " << message.header();
                    error_handler_.on_error(error_message.str());
                }
        }

        ConnectionState new_state = transition_incoming(state_, message);

        if (new_state == ConnectionState::ERROR) {
            std::stringstream error_message;
            error_message << "connection error state reached, previous state: " << state_
                << ", message header: " << message.header();
            error_handler_.on_error(error_message.str());
        }

        state(new_state);
    }

    void Connection::on_error(const std::string &&error)
    {
        DEBUG_MSG("Websocket error: " << error);
        on_close();
    }

    void Connection::on_open()
    {
        reconnection_attempt_ = 0;
        state(ConnectionState::AWAIT_CONNECTION);
    }

    void Connection::on_close()
    {
        if (deliberate_close_ || reconnection_attempt_ >= 3) {
            state(ConnectionState::CLOSED);
            return;
        }
        reconnection_attempt_++;
        state(ConnectionState::RECONNECTING);
        ws_handler_.open();
    }

    bool Connection::send(const Message& message)
    {
        DEBUG_MSG("--> Sending message: " << message.header());

        if (message.topic() == Topic::CONNECTION || message.topic() == Topic::AUTH) {
            ConnectionState new_state = transition_outgoing(state_, message);
            assert(new_state != ConnectionState::ERROR);

            if (new_state == ConnectionState::ERROR) {
                throw std::logic_error("Invalid client state transition");
            }

            state(new_state);
        } else if (state_ != ConnectionState::OPEN) {
            return false;
        }

        return ws_handler_.send(message.to_binary());
    }

    ConnectionState transition_incoming(const ConnectionState state, const Message& message)
    {
        assert(state != ConnectionState::ERROR);

        const Topic topic = message.topic();
        const Action action = message.action();
        const bool is_ack = message.is_ack();

        const auto expected_num_args = Message::num_arguments(message.header());
        const std::size_t& num_args = message.num_arguments();

        use(expected_num_args);
        use(num_args);
        assert(num_args >= expected_num_args.first);
        assert(num_args <= expected_num_args.second);

        // deepstream ping/pong (distinct from websocket heartbeats)
        if (topic == Topic::CONNECTION && action == Action::PING) {
            assert(!is_ack);

            return state;
        }

        // actual state transitions
        if (state == ConnectionState::AWAIT_CONNECTION && topic == Topic::CONNECTION
                && action == Action::CHALLENGE) {
            assert(!is_ack);

            return ConnectionState::CHALLENGING;
        }

        if (state == ConnectionState::CHALLENGING_WAIT && topic == Topic::CONNECTION
                && action == Action::CHALLENGE_RESPONSE) {
            assert(is_ack);
            return ConnectionState::AWAIT_AUTHENTICATION;
        }

        if (state == ConnectionState::CHALLENGING_WAIT && topic == Topic::CONNECTION
                && action == Action::REDIRECT) {
            assert(!is_ack);

            return ConnectionState::AWAIT_CONNECTION;
        }

        if (state == ConnectionState::CHALLENGING_WAIT && topic == Topic::CONNECTION
                && action == Action::REJECT) {
            assert(!is_ack);

            return ConnectionState::CLOSED;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH
                && action == Action::REQUEST) {
            assert(is_ack);
            return ConnectionState::OPEN;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH
                && action == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS) {
            assert(!is_ack);

            return ConnectionState::CLOSED;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH
                && action == Action::ERROR_INVALID_AUTH_DATA) {
            assert(!is_ack);

            return ConnectionState::AWAIT_AUTHENTICATION;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH
                && action == Action::ERROR_INVALID_AUTH_MSG) {
            assert(!is_ack);

            return ConnectionState::CLOSED;
        }

        if (state == ConnectionState::OPEN) {
            return state;
        }

        return ConnectionState::ERROR;
    }

    ConnectionState transition_outgoing(const ConnectionState state, const Message& message)
    {
        assert(state != ConnectionState::ERROR);
        assert(state != ConnectionState::CLOSED);

        const Topic topic = message.topic();
        const Action action = message.action();
        const bool is_ack = message.is_ack();

        const auto expected_num_args = Message::num_arguments(message.header());
        const std::size_t& num_args = message.num_arguments();

        use(expected_num_args);
        use(num_args);
        assert(num_args >= expected_num_args.first);
        assert(num_args <= expected_num_args.second);

        if (topic == Topic::CONNECTION && action == Action::PONG) {
            assert(!is_ack);

            return state;
        }

        if (state == ConnectionState::CHALLENGING && topic == Topic::CONNECTION
                && action == Action::CHALLENGE_RESPONSE) {
            assert(!is_ack);
            return ConnectionState::CHALLENGING_WAIT;
        }

        if (state == ConnectionState::AWAIT_AUTHENTICATION && topic == Topic::AUTH
                && action == Action::REQUEST) {
            assert(!is_ack);
            return ConnectionState::AUTHENTICATING;
        }

        return ConnectionState::ERROR;
    }
}
