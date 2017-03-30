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

#include <cstdint>
#include <cstring>

#include <arpa/inet.h>

#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>
#include <deepstream/core/error_handler.hpp>
#include <deepstream/core/ws.hpp>

#include "../connection.hpp"
#include "../message_builder.hpp"
#include "../parser.hpp"
#include "../state.hpp"

#include <cassert>

namespace deepstream {

    struct FailHandler : public ErrorHandler {
        virtual void on_error(const std::string &) const override
        {
            BOOST_FAIL("There should be no errors");
        }
    };

    struct SimpleClient : public Client {
         SimpleClient() : Client()
         {
         }
    }
    /*
     *struct SimpleClient : public websockets::pseudo::Client {
     *    typedef std::unique_ptr<websockets::Frame> FramePtr;
     *
     *    SimpleClient()
     *        : state_(State::AWAIT_CONNECTION)
     *    {
     *    }
     *
     *    virtual std::unique_ptr<websockets::WebSocketClient>
     *    construct_impl(const std::string&) const override
     *    {
     *        BOOST_FAIL("This method should not be called");
     *        return nullptr;
     *    }
     *
     *    std::pair<websockets::State, FramePtr> f(Topic t, Action a,
     *        bool ack = false)
     *    {
     *        Message::Header header(t, a, ack);
     *        auto expected_num_args = Message::num_arguments(header);
     *
     *        MessageBuilder builder(header);
     *
     *        for (std::size_t i = 0; i < expected_num_args.first; ++i)
     *            builder.add_argument("arg");
     *
     *        state_ = transition(state_, builder, Sender::SERVER);
     *
     *        return std::make_pair(websockets::State::OPEN,
     *            builder.to_binary());
     *    };
     *
     *    virtual std::pair<websockets::State, FramePtr> receive_frame_impl() override
     *    {
     *        if (state_ == State::AWAIT_CONNECTION)
     *            return f(Topic::CONNECTION, Action::CHALLENGE);
     *        if (state_ == State::CHALLENGING_WAIT)
     *            return f(Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true);
     *        if (state_ == State::AUTHENTICATING)
     *            return f(Topic::AUTH, Action::REQUEST, true);
     *        if (state_ == State::CONNECTED) {
     *            using Frame = websockets::Frame;
     *
     *            const std::uint16_t NORMAL_CLOSE = 1006;
     *            const std::size_t size = sizeof(std::uint16_t);
     *
     *            union {
     *                std::uint16_t as_uint16_t;
     *                char as_char[size];
     *            } payload;
     *            payload.as_uint16_t = htons(NORMAL_CLOSE);
     *
     *            return std::make_pair(
     *                websockets::State::OPEN,
     *                FramePtr(
     *                    new Frame(Frame::Bit::FIN | Frame::Opcode::CONNECTION_CLOSE_FRAME,
     *                        payload.as_char, size)));
     *        }
     *
     *        BOOST_FAIL("Improper state transition detected");
     *        return std::make_pair(websockets::State::ERROR, nullptr);
     *    }
     *
     *    virtual websockets::State send_frame_impl(const Buffer& frame,
     *        websockets::Frame::Flags) override
     *    {
     *        Buffer input(frame);
     *        input.push_back(0);
     *        input.push_back(0);
     *
     *        auto parser_retvals = parser::execute(input.data(), input.size());
     *        const parser::MessageList& messages = parser_retvals.first;
     *        const parser::ErrorList& errors = parser_retvals.second;
     *
     *        BOOST_REQUIRE(errors.empty());
     *
     *        std::for_each(messages.cbegin(), messages.cend(),
     *            [this](const Message& msg) {
     *                State old_state = this->state_;
     *                State new_state = transition(old_state, msg, Sender::CLIENT);
     *
     *                this->state_ = new_state;
     *            });
     *
     *        return websockets::State::OPEN;
     *    }
     *
     *    State state_;
     *};
     */

    struct SimpleWSHandler : public WSHandler {
        SimpleWSHandler() : WSHandler()
        {
        }

        ~SimpleWSHandler()
        {
        }

        void process_messages() {}

        std::string URI() const override {
            return "test_uri";
        }

        void URI(std::string) override {}

        void send(const Buffer&) override {}

        void open() override {}

        void close() override {}

        void reconnect() override {}

        void shutdown() override {}

        void on_open(const HandlerFn&) override {}

        void on_close(const HandlerFn&) override {}

        void on_error(const HandlerWithMsgFn&) override {}

        void on_message(const HandlerWithBufFn&) override {}

        WSState state() const override
        {
            return WSState::CLOSED;
        }
    };

    BOOST_AUTO_TEST_CASE(simple)
    {
        SimpleWSHandler wsh;
        FailHandler errh;
        Connection conn("ws://uri", wsh, errh);

        conn.login("auth", [](const deepstream::Buffer &){});

        BOOST_CHECK_EQUAL(conn.get_connection_state(), ConnectionState::CONNECTED);
    }

    /*
     *struct RedirectionClient : public websockets::pseudo::Client {
     *    typedef std::unique_ptr<websockets::Frame> FramePtr;
     *
     *    static constexpr const char DEFAULT_URI[] = "ws://default-url";
     *    static constexpr const char REDIRECTION_URI[] = "ws://redirect-url";
     *
     *    RedirectionClient(const std::string& uri = DEFAULT_URI,
     *        bool do_redirect = true)
     *        : state_(ConnectionState::AWAIT_CONNECTION)
     *        , do_redirect_(do_redirect)
     *        , uri_(uri)
     *    {
     *    }
     *
     *    virtual std::string uri_impl() const override { return uri_; }
     *
     *    virtual std::unique_ptr<websockets::WebSocketClient>
     *    construct_impl(const std::string& uri) const override
     *    {
     *        BOOST_CHECK(do_redirect_);
     *        BOOST_CHECK_EQUAL(uri, REDIRECTION_URI);
     *
     *        return std::unique_ptr<websockets::WebSocketClient>(
     *            new RedirectionClient(REDIRECTION_URI, false));
     *    }
     *
     *    std::pair<websockets::State, FramePtr> f(Topic t, Action a,
     *        bool ack = false)
     *    {
     *        Message::Header header(t, a, ack);
     *
     *        MessageBuilder builder(header);
     *
     *        if (a == Action::REDIRECT)
     *            builder.add_argument(REDIRECTION_URI);
     *
     *        state_ = transition(state_, builder, Sender::SERVER);
     *
     *        return std::make_pair(websockets::State::OPEN,
     *            make_frame(builder.to_binary()));
     *    };
     *
     *    virtual std::pair<websockets::State, FramePtr> receive_frame_impl() override
     *    {
     *        if (state_ == ConnectionState::AWAIT_CONNECTION)
     *            return f(Topic::CONNECTION, Action::CHALLENGE);
     *        if (state_ == ConnectionState::CHALLENGING_WAIT && do_redirect_)
     *            return f(Topic::CONNECTION, Action::REDIRECT);
     *        if (state_ == ConnectionState::CHALLENGING_WAIT && !do_redirect_)
     *            return f(Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true);
     *        if (state_ == ConnectionState::AUTHENTICATING)
     *            return f(Topic::AUTH, Action::REQUEST, true);
     *        if (state_ == ConnectionState::CONNECTED) {
     *            using Frame = websockets::Frame;
     *
     *            const std::uint16_t NORMAL_CLOSE = 1006;
     *            const std::size_t size = sizeof(std::uint16_t);
     *
     *            union {
     *                std::uint16_t as_uint16_t;
     *                char as_char[size];
     *            } payload;
     *            payload.as_uint16_t = htons(NORMAL_CLOSE);
     *
     *            return std::make_pair(
     *                websockets::State::OPEN,
     *                FramePtr(
     *                    new Frame(Frame::Bit::FIN | Frame::Opcode::CONNECTION_CLOSE_FRAME,
     *                        payload.as_char, size)));
     *        }
     *
     *        BOOST_FAIL("Improper state transition detected");
     *        return std::make_pair(websockets::State::ERROR, nullptr);
     *    }
     *
     *    virtual websockets::State send_frame_impl(const Buffer& frame,
     *        websockets::Frame::Flags) override
     *    {
     *        Buffer input(frame);
     *        input.push_back(0);
     *        input.push_back(0);
     *
     *        auto parser_retvals = parser::execute(input.data(), input.size());
     *        const parser::MessageList& messages = parser_retvals.first;
     *        const parser::ErrorList& errors = parser_retvals.second;
     *
     *        BOOST_REQUIRE(errors.empty());
     *
     *        std::for_each(messages.cbegin(), messages.cend(),
     *            [this](const Message& msg) {
     *                ConnectionState old_state = this->state_;
     *                ConnectionState new_state = transition(old_state, msg, Sender::CLIENT);
     *
     *                this->state_ = new_state;
     *            });
     *
     *        return websockets::State::OPEN;
     *    }
     *
     *    ConnectionState state_;
     *    bool do_redirect_;
     *    std::string uri_;
     *};
     */

    struct RedirectionWSHandler : public WSHandler {
        RedirectionWSHandler() : WSHandler()
        {
        }

        ~RedirectionWSHandler()
        {
        }

        void process_messages() {}

        std::string URI() const override {
            return "test_uri";
        }

        void URI(std::string) override {}

        void send(const Buffer&) override {}

        void open() override {}

        void close() override {}

        void reconnect() override {}

        void shutdown() override {}

        void on_open(const HandlerFn&) override {}

        void on_close(const HandlerFn&) override {}

        void on_error(const HandlerWithMsgFn&) override {}

        void on_message(const HandlerWithBufFn&) override {}

        WSState state() const override
        {
            return WSState::CLOSED;
        }
    };

    /*
     *constexpr const char RedirectionClient::DEFAULT_URI[];
     *constexpr const char RedirectionClient::REDIRECTION_URI[];
     */

    BOOST_AUTO_TEST_CASE(redirections)
    {
        RedirectionWSHandler wsh;
        FailHandler errh;
        Connection conn("ws://initial.uri", wsh, errh);

        conn.login("auth", [](const deepstream::Buffer &){});

        BOOST_CHECK_EQUAL(conn.get_connection_state(), ConnectionState::CONNECTED);
        BOOST_CHECK_EQUAL(wsh.URI(), "ws://redirection.uri");
    }
}
