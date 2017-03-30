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
#include "state.hpp"
#include "use.hpp"
#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>
#include <deepstream/core/error_handler.hpp>
#include <deepstream/core/event.hpp>
#include <deepstream/core/presence.hpp>

#include <cassert>

#ifndef NDEBUG
#include <ostream>
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

namespace deepstream {

    std::ostream &operator<<(std::ostream &os, ConnectionState state) {
        os << static_cast<int>(state);
        return os;
    }

    Connection::Connection(Client &client, const std::string &uri, WSHandler &ws_handler, ErrorHandler &error_handler)
        : client_(client)
        , state_(ConnectionState::CLOSED)
        , error_handler_(error_handler)
        , ws_handler_(ws_handler)
        , p_login_callback_(nullptr)
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

    /*
     *
     *std::unique_ptr<Connection>
     *Connection::make(std::unique_ptr<websockets::WebSocketClient> p_websocket,
     *    std::unique_ptr<ErrorHandler> p_error_handler, WSHandler &ws_handler)
     *{
     *    assert(p_websocket);
     *    assert(p_error_handler);
     *
     *    const unsigned MAX_NUM_REDIRECTIONS = 3;
     *
     *    for (unsigned num_redirections = 0; num_redirections < MAX_NUM_REDIRECTIONS;
     *         ++num_redirections) {
     *        p_websocket->set_receive_timeout(std::chrono::seconds(10));
     *
     *        const std::string uri = p_websocket->uri();
     *
     *        std::unique_ptr<Connection> p(new Connection(std::move(p_websocket), std::move(p_error_handler)));
     *
     *        Buffer buffer;
     *        parser::MessageList messages;
     *        ConnectionState& state = p->state_;
     *
     *        if (p->receive_(&buffer, &messages) != websockets::State::OPEN)
     *            return p;
     *
     *        if (messages.size() != 1 || state != ConnectionState::CHALLENGING) {
     *            p->close();
     *            return p;
     *        }
     *
     *        {
     *            MessageBuilder chr(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
     *            chr.add_argument(uri);
     *
     *            if (p->send_(chr) != websockets::State::OPEN)
     *                return p;
     *        }
     *
     *        if (p->receive_(&buffer, &messages) != websockets::State::OPEN)
     *            return p;
     *
     *        if (messages.size() != 1) {
     *            p->close();
     *            return p;
     *        }
     *
     *        if (state == ConnectionState::AWAIT_AUTHENTICATION)
     *            return p;
     *
     *        if (state == ConnectionState::AWAIT_CONNECTION) {
     *            const Message& redirect_msg = messages.front();
     *            assert(redirect_msg.num_arguments() == 1);
     *
     *            const Buffer& uri_buffer = redirect_msg[0];
     *            std::string new_uri(uri_buffer.cbegin(), uri_buffer.cend());
     *
     *            p_websocket = p->p_websocket_->construct(new_uri);
     *            p_error_handler = std::move(p->p_error_handler_);
     *
     *            continue;
     *        }
     *
     *        p->close();
     *        return p;
     *    }
     *
     *    assert(p_error_handler);
     *    p_error_handler->onError(ErrorState::TOO_MANY_REDIRECTIONS);
     *
     *    return std::unique_ptr<Connection>(
     *        new Connection(nullptr, std::move(p_error_handler)));
     *}
     *
     *std::unique_ptr<Connection> Connection::make(const std::string& uri,
     *    std::unique_ptr<ErrorHandler> p_eh, WSHandler &ws_handler)
     *{
     *    if (uri.empty())
     *        throw std::invalid_argument("URI must not be empty");
     *
     *    //return Client::make(std::unique_ptr<websockets::Client>(websockets::poco::Client::makeClient(uri)), std::move(p_eh));
     *    return nullptr;
     *}
     */

    void Connection::login(const std::string& auth, const Client::LoginCallback &callback)
    {
        if (state_ != ConnectionState::AWAIT_AUTHENTICATION) {
            throw std::logic_error("Cannot login() in current state");
        }

        p_login_callback_ = std::unique_ptr<Client::LoginCallback>(new Client::LoginCallback(callback));

        MessageBuilder authentication_request(Topic::AUTH, Action::REQUEST);
        authentication_request.add_argument(auth);

        send(authentication_request);

        Buffer buffer;
        parser::MessageList messages;

        /*
         *    if (receive_(&buffer, &messages) != websockets::State::OPEN) {
         *        return false;
         *    }
         *
         *    if (messages.size() != 1) {
         *        close();
         *        return false;
         *    }
         *
         *    const Message& msg = messages.front();
         *
         *    if (msg.topic() == Topic::AUTH && msg.action() == Action::REQUEST && msg.is_ack()) {
         *        assert(state_ == ConnectionState::OPEN);
         *
         *        if (msg.num_arguments() == 0 && p_user_data)
         *            p_user_data->clear();
         *        else if (msg.num_arguments() == 1 && p_user_data)
         *            *p_user_data = msg[0];
         *
         *        return true;
         *    }
         *
         *    if (msg.topic() == Topic::AUTH && msg.action() == Action::ERROR_INVALID_AUTH_DATA) {
         *        assert(state_ == ConnectionState::AWAIT_AUTHENTICATION);
         *        p_error_handler_->onError(ErrorState::AUTHENTICATION_ERROR);
         *        return false;
         *    }
         *
         *    if (msg.topic() == Topic::AUTH && msg.action() == Action::ERROR_INVALID_AUTH_MSG) {
         *        p_error_handler_->onError(ErrorState::AUTHENTICATION_ERROR);
         *        close();
         *        return false;
         *    }
         *
         *    if (msg.topic() == Topic::AUTH && msg.action() == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS) {
         *        p_error_handler_->onError(ErrorState::AUTHENTICATION_ERROR);
         *        close();
         *        return false;
         *    }
         *
         *    close();
         *    return false;
         */
    }

    void Connection::close()
    {
        ws_handler_.close();
        state_ = ConnectionState::CLOSED;
    }

    ConnectionState Connection::get_connection_state() { return state_; }

    void Connection::on_message(const Buffer &raw_message)
    {
        Buffer buffer;

        buffer.assign(raw_message.cbegin(), raw_message.cend());
        buffer.push_back(0);
        buffer.push_back(0);

        auto parser_result = parser::execute(buffer.data(), buffer.size());
        const parser::ErrorList& errors = parser_result.second;

        for (auto it = errors.cbegin(); it != errors.cend(); ++it) {
            const parser::Error &error = *it;
            std::stringstream error_message;
            error_message << "parser error: " << error;
            on_error(error_message.str());
        }

        if (!errors.empty()) {
            return;
        }

        parser::MessageList parsed_messages(std::move(parser_result.first));

        for (auto it = parsed_messages.cbegin(); it != parsed_messages.cend(); ++it) {
            const Message &parsed_message = *it;

            switch (parsed_message.topic()) {
                case Topic::EVENT:
                    client_.event.notify_(parsed_message);
                    break;

                case Topic::PRESENCE:
                    client_.presence.notify_(parsed_message);
                    break;

                case Topic::CONNECTION:
                case Topic::AUTH:
                    handle_connection_response(parsed_message);
                    break;

                default:
                    on_error("unsolicited message");
                    assert(0);
            }

        }
    }

    void Connection::handle_connection_response(const Message &message)
    {
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
            case Action::REDIRECT:
                {
                    if (message.num_arguments() < 1) {
                        on_error("No URI given in connection redirect message");
                        break;
                    }
                    ws_handler_.close();
                    const Buffer &uri_buff(message[0]);
                    ws_handler_.URI(std::string(uri_buff.cbegin(), uri_buff.cend()));
                    ws_handler_.open();
                } break;
            default:
                {
                    std::stringstream error_message;
                    error_message << "connection message with unknown action" << state_
                        << ", message header: " << message.header();
                    on_error(error_message.str());
                }
        }

        ConnectionState new_state = transition_incoming(state_, message);

        if (new_state == ConnectionState::ERROR) {
            std::stringstream error_message;
            error_message << "connection error state reached, previous state: " << state_
                << ", message header: " << message.header();
            on_error(error_message.str());
        }

        state_ = new_state;
    }

    void Connection::handle_authentication_response(const Message &message)
    {
        switch(message.action()) {
            case Action::ERROR_TOO_MANY_AUTH_ATTEMPTS:
                {
                    on_error("too many authentication attempts");
                    close();
                } break;
            case Action::ERROR_INVALID_AUTH_DATA:
                {
                } break;
            case Action::ERROR_INVALID_AUTH_MSG:
                {
                } break;
            case Action::REQUEST:
                {
                    assert(message.is_ack());
                    //TODO: flush message buffer
                    if (p_login_callback_) {
                        const auto login_callback = *p_login_callback_;
                        if (message.num_arguments() < 1) {
                            login_callback(std::unique_ptr<Buffer>(nullptr));
                        } else {
                            const std::unique_ptr<Buffer> auth_data(std::unique_ptr<Buffer>(new Buffer(message[0])));
                            login_callback(auth_data);
                        }
                    }
                } break;
            default:
                {
                    std::stringstream error_message;
                    error_message << "connection message with unknown action" << state_
                        << ", message header: " << message.header();
                    on_error(error_message.str());
                }
        }

        ConnectionState new_state = transition_incoming(state_, message);

        if (new_state == ConnectionState::ERROR) {
            std::stringstream error_message;
            error_message << "connection error state reached, previous state: " << state_
                << ", message header: " << message.header();
            on_error(error_message.str());
        }

        state_ = new_state;
    }

    void Connection::on_error(const std::string &error)
    {
        state_ = ConnectionState::ERROR;
    }

    void Connection::on_open()
    {
        state_ = ConnectionState::AWAIT_CONNECTION;
    }

    void Connection::on_close()
    {
    }


    /*
     *void Connection::process_messages(Event* p_event, Presence* p_presence)
     *{
     *    assert(p_event);
     *    assert(p_presence);
     *
     *    if (!p_websocket_)
     *        return;
     *
     *    Buffer buffer;
     *    parser::MessageList messages;
     *
     *    p_websocket_->set_receive_timeout(std::chrono::milliseconds(1));
     *
     *    while (receive_(&buffer, &messages) == websockets::State::OPEN) {
     *        if (messages.empty())
     *            break;
     *
     *        for (const Message& message : messages) {
     *            if (message.topic() == Topic::CONNECTION && message.action() == Action::PING) {
     *                MessageBuilder pong(Topic::CONNECTION, Action::PONG);
     *                send(pong);
     *                continue;
     *            }
     *
     *            switch (message.topic()) {
     *            case Topic::EVENT:
     *                p_event->notify_(message);
     *                break;
     *
     *            case Topic::PRESENCE:
     *                p_presence->notify_(message);
     *                break;
     *
     *            default:
     *                assert(0);
     *                close();
     *                return;
     *            }
     *        }
     *    }
     *}
     */

    /*
     *websockets::State Connection::receive_(Buffer* p_buffer,
     *    parser::MessageList* p_messages)
     *{
     *    assert(p_buffer);
     *    assert(p_messages);
     *
     *    if (!p_websocket_)
     *        return websockets::State::ERROR;
     *
     *    auto receive_ret = p_websocket_->receive_frame();
     *    websockets::State ws_state = receive_ret.first;
     *    std::unique_ptr<websockets::Frame> p_frame = std::move(receive_ret.second);
     *
     *    if (ws_state == websockets::State::OPEN && !p_frame) {
     *        p_buffer->clear();
     *        p_messages->clear();
     *
     *        return ws_state;
     *    }
     *
     *    if (ws_state == websockets::State::CLOSED && p_frame) {
     *        p_buffer->clear();
     *        p_messages->clear();
     *
     *        close();
     *
     *        return ws_state;
     *    }
     *
     *    if (ws_state == websockets::State::CLOSED && !p_frame) {
     *        p_buffer->clear();
     *        p_messages->clear();
     *
     *        close();
     *        p_error_handler_->onError(ErrorState::SUDDEN_DISCONNECT);
     *
     *        return ws_state;
     *    }
     *
     *    if (ws_state == websockets::State::ERROR) {
     *        assert(p_frame);
     *
     *        close();
     *        p_error_handler_->onError(ErrorState::INVALID_CLOSE_FRAME_SIZE);
     *
     *        return ws_state;
     *    }
     *
     *    assert(ws_state == websockets::State::OPEN);
     *    assert(p_frame);
     *
     *    if (p_frame->flags() != (websockets::Frame::Bit::FIN | websockets::Frame::Opcode::TEXT_FRAME)) {
     *        p_error_handler_->onError(ErrorState::UNEXPECTED_WEBSOCKET_FRAME_FLAGS);
     *        close();
     *
     *        return websockets::State::ERROR;
     *    }
     *
     *    const Buffer& payload = p_frame->payload();
     *    p_buffer->assign(payload.cbegin(), payload.cend());
     *    p_buffer->push_back(0);
     *    p_buffer->push_back(0);
     *
     *    auto parser_ret = parser::execute(p_buffer->data(), p_buffer->size());
     *    const parser::ErrorList& errors = parser_ret.second;
     *
     *    std::for_each(errors.cbegin(), errors.cend(), [this](const parser::Error&) {
     *        this->p_error_handler_->onError(ErrorState::PARSER_ERROR);
     *    });
     *
     *    if (!errors.empty()) {
     *        p_buffer->clear();
     *        p_messages->clear();
     *
     *        close();
     *        return websockets::State::ERROR;
     *    }
     *
     *    *p_messages = std::move(parser_ret.first);
     *
     *    for (auto it = p_messages->cbegin(); it != p_messages->cend(); ++it) {
     *        const Message& msg = *it;
     *
     *        ConnectionState old_state = state_;
     *        ConnectionState new_state = transition(old_state, msg, Sender::SERVER);
     *
     *        if (new_state == ConnectionState::ERROR) {
     *            close();
     *            p_error_handler_->onError(ErrorState::INVALID_STATE_TRANSITION);
     *
     *            return websockets::State::ERROR;
     *        }
     *
     *        if (new_state == ConnectionState::CLOSED)
     *            return websockets::State::OPEN;
     *
     *        state_ = new_state;
     *    }
     *
     *    return ws_state;
     *}
     */

    void Connection::send(const Message& message)
    {
        ConnectionState new_state = transition_outgoing(state_, message);
        assert(new_state != ConnectionState::ERROR);

        if (new_state == ConnectionState::ERROR)
            throw std::logic_error("Invalid client state transition");

        state_ = new_state;

        return ws_handler_.send(message.to_binary());
    }


    ConnectionState Connection::transition_incoming(ConnectionState state, const Message& message)
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

        // deepstream ping/pong (distinct from websocket heartbeats)
        if (topic == Topic::CONNECTION && action == Action::PING) {
            assert(!is_ack);

            return state;
        }

        // actual state transitions
        if (state == ConnectionState::AWAIT_CONNECTION && topic == Topic::CONNECTION && action == Action::CHALLENGE) {
            assert(!is_ack);

            return ConnectionState::CHALLENGING;
        }

        if (state == ConnectionState::RECONNECTING && topic == Topic::CONNECTION && action == Action::CHALLENGE_RESPONSE && is_ack) {
            return ConnectionState::AWAIT_AUTHENTICATION;
        }

        if (state == ConnectionState::RECONNECTING && topic == Topic::CONNECTION && action == Action::REDIRECT) {
            assert(!is_ack);

            return ConnectionState::AWAIT_CONNECTION;
        }

        if (state == ConnectionState::RECONNECTING && topic == Topic::CONNECTION && action == Action::REJECT) {
            assert(!is_ack);

            return ConnectionState::CLOSED;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::REQUEST && is_ack) {
            return ConnectionState::OPEN;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS) {
            assert(!is_ack);

            return ConnectionState::CLOSED;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_INVALID_AUTH_DATA) {
            assert(!is_ack);

            return ConnectionState::AWAIT_AUTHENTICATION;
        }

        if (state == ConnectionState::AUTHENTICATING && topic == Topic::AUTH && action == Action::ERROR_INVALID_AUTH_MSG) {
            assert(!is_ack);

            return ConnectionState::CLOSED;
        }

        if (state == ConnectionState::OPEN) {
            return state;
        }

        return ConnectionState::ERROR;
    }

    ConnectionState Connection::transition_outgoing(ConnectionState state, const Message& message)
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

        if (state == ConnectionState::CHALLENGING && topic == Topic::CONNECTION && action == Action::CHALLENGE_RESPONSE && !is_ack) {
            return ConnectionState::RECONNECTING;
        }

        if (state == ConnectionState::AWAIT_AUTHENTICATION && topic == Topic::AUTH && action == Action::REQUEST && !is_ack) {
            return ConnectionState::AUTHENTICATING;
        }

        if (state == ConnectionState::OPEN)
            return state;

        return ConnectionState::ERROR;
    }
}
