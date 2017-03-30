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

    Connection::Connection(const std::string &uri, WSHandler &ws_handler, ErrorHandler &error_handler)
        : state_(ConnectionState::AWAIT_CONNECTION)
          , error_handler_(error_handler)
          , ws_handler_(ws_handler)
          , p_login_callback_(nullptr)
    {
        assert(ws_handler.state() == WSState::CLOSED);

        ws_handler.URI(uri);
        ws_handler.on_message(std::bind(&Connection::on_message, this, std::placeholders::_1));
        ws_handler.on_error(std::bind(&Connection::on_error, this, std::placeholders::_1));
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

        send_(authentication_request);

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
         *        assert(state_ == ConnectionState::CONNECTED);
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
        // state_ = ConnectionState::DISCONNECTED;
    }

    ConnectionState Connection::get_connection_state() { return state_; }

    void Connection::on_message(const Buffer &message)
    {
        // TODO: parse messages

        // TODO: call message handlers
    }

    void Connection::on_error(const std::string &error)
    {
    }

    void Connection::on_open()
    {
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
     *                send_(pong);
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
     *        if (new_state == ConnectionState::DISCONNECTED)
     *            return websockets::State::OPEN;
     *
     *        state_ = new_state;
     *    }
     *
     *    return ws_state;
     *}
     */

    void Connection::send_(const Message& message)
    {
        ConnectionState new_state = transition(state_, message, Sender::CLIENT);
        assert(new_state != ConnectionState::ERROR);

        if (new_state == ConnectionState::ERROR)
            throw std::logic_error("Invalid client state transition");

        state_ = new_state;

        return ws_handler_.send(message.to_binary());
    }
}
