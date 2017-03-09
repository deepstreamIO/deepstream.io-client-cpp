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

#include <deepstream/buffer.hpp>
#include <deepstream/client.hpp>
#include <deepstream/error_handler.hpp>
#include "../src/impl.hpp"
#include <deepstream/message_builder.hpp>
#include <deepstream/parser.hpp>
#include "../src/websockets.hpp"
#include "../src/websockets/pseudo.hpp"

#include <cassert>

namespace deepstream {
namespace impl {

    struct FailHandler : public ErrorHandler {
        virtual void parser_error_impl(const parser::Error&) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void invalid_state_transition_impl(client::State,
            const Message&) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void system_error_impl(const std::system_error&) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void
        invalid_close_frame_size_impl(const websockets::Frame&) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void websocket_exception_impl(const std::exception&) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void unexpected_websocket_frame_flags_impl(int) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void sudden_disconnect_impl(const std::string&) override
        {
            BOOST_FAIL("There should be no errors");
        }

        virtual void authentication_error_impl(const Message&) override
        {
            BOOST_FAIL("There should be no errors");
        }
    };

    std::unique_ptr<websockets::Frame> make_frame(const Buffer& buffer)
    {
        using Frame = websockets::Frame;

        return std::unique_ptr<Frame>(
            new Frame(Frame::Bit::FIN | Frame::Opcode::TEXT_FRAME, buffer.data(),
                buffer.size()));
    }

    struct SimpleClient : public websockets::pseudo::Client {
        typedef std::unique_ptr<websockets::Frame> FramePtr;

        SimpleClient()
            : state_(client::State::AWAIT_CONNECTION)
        {
        }

        virtual std::unique_ptr<websockets::Client>
        construct_impl(const std::string&) const override
        {
            BOOST_FAIL("This method should not be called");
            return nullptr;
        }

        std::pair<websockets::State, FramePtr> f(Topic t, Action a,
            bool ack = false)
        {
            Message::Header header(t, a, ack);
            auto expected_num_args = Message::num_arguments(header);

            MessageBuilder builder(header);

            for (std::size_t i = 0; i < expected_num_args.first; ++i)
                builder.add_argument("arg");

            state_ = client::transition(state_, builder, Sender::SERVER);

            return std::make_pair(websockets::State::OPEN,
                make_frame(builder.to_binary()));
        };

        virtual std::pair<websockets::State, FramePtr> receive_frame_impl() override
        {
            using State = client::State;

            if (state_ == State::AWAIT_CONNECTION)
                return f(Topic::CONNECTION, Action::CHALLENGE);
            if (state_ == State::CHALLENGING_WAIT)
                return f(Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true);
            if (state_ == State::AUTHENTICATING)
                return f(Topic::AUTH, Action::REQUEST, true);
            if (state_ == State::CONNECTED) {
                using Frame = websockets::Frame;

                const std::uint16_t NORMAL_CLOSE = 1006;
                const std::size_t size = sizeof(std::uint16_t);

                union {
                    std::uint16_t as_uint16_t;
                    char as_char[size];
                } payload;
                payload.as_uint16_t = htons(NORMAL_CLOSE);

                return std::make_pair(
                    websockets::State::OPEN,
                    FramePtr(
                        new Frame(Frame::Bit::FIN | Frame::Opcode::CONNECTION_CLOSE_FRAME,
                            payload.as_char, size)));
            }

            BOOST_FAIL("Improper state transition detected");
            return std::make_pair(websockets::State::ERROR, nullptr);
        }

        virtual websockets::State send_frame_impl(const Buffer& frame,
            websockets::Frame::Flags) override
        {
            Buffer input(frame);
            input.push_back(0);
            input.push_back(0);

            auto parser_retvals = parser::execute(input.data(), input.size());
            const parser::MessageList& messages = parser_retvals.first;
            const parser::ErrorList& errors = parser_retvals.second;

            BOOST_REQUIRE(errors.empty());

            std::for_each(messages.cbegin(), messages.cend(),
                [this](const Message& msg) {
                    client::State old_state = this->state_;
                    client::State new_state = client::transition(old_state, msg, Sender::CLIENT);

                    this->state_ = new_state;
                });

            return websockets::State::OPEN;
        }

        client::State state_;
    };

    BOOST_AUTO_TEST_CASE(simple)
    {
        std::unique_ptr<Client> p = Client::make(std::unique_ptr<websockets::Client>(new SimpleClient),
            std::unique_ptr<ErrorHandler>(new FailHandler));

        p->login("auth", nullptr);

        BOOST_CHECK_EQUAL(p->getConnectionState(), client::State::CONNECTED);
    }

    struct RedirectionClient : public websockets::pseudo::Client {
        typedef std::unique_ptr<websockets::Frame> FramePtr;

        static constexpr const char DEFAULT_URI[] = "ws://default-url";
        static constexpr const char REDIRECTION_URI[] = "ws://redirect-url";

        RedirectionClient(const std::string& uri = DEFAULT_URI,
            bool do_redirect = true)
            : state_(client::State::AWAIT_CONNECTION)
            , do_redirect_(do_redirect)
            , uri_(uri)
        {
        }

        virtual std::string uri_impl() const override { return uri_; }

        virtual std::unique_ptr<websockets::Client>
        construct_impl(const std::string& uri) const override
        {
            BOOST_CHECK(do_redirect_);
            BOOST_CHECK_EQUAL(uri, REDIRECTION_URI);

            return std::unique_ptr<websockets::Client>(
                new RedirectionClient(REDIRECTION_URI, false));
        }

        std::pair<websockets::State, FramePtr> f(Topic t, Action a,
            bool ack = false)
        {
            Message::Header header(t, a, ack);

            MessageBuilder builder(header);

            if (a == Action::REDIRECT)
                builder.add_argument(REDIRECTION_URI);

            state_ = client::transition(state_, builder, Sender::SERVER);

            return std::make_pair(websockets::State::OPEN,
                make_frame(builder.to_binary()));
        };

        virtual std::pair<websockets::State, FramePtr> receive_frame_impl() override
        {
            using State = client::State;

            if (state_ == State::AWAIT_CONNECTION)
                return f(Topic::CONNECTION, Action::CHALLENGE);
            if (state_ == State::CHALLENGING_WAIT && do_redirect_)
                return f(Topic::CONNECTION, Action::REDIRECT);
            if (state_ == State::CHALLENGING_WAIT && !do_redirect_)
                return f(Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true);
            if (state_ == State::AUTHENTICATING)
                return f(Topic::AUTH, Action::REQUEST, true);
            if (state_ == State::CONNECTED) {
                using Frame = websockets::Frame;

                const std::uint16_t NORMAL_CLOSE = 1006;
                const std::size_t size = sizeof(std::uint16_t);

                union {
                    std::uint16_t as_uint16_t;
                    char as_char[size];
                } payload;
                payload.as_uint16_t = htons(NORMAL_CLOSE);

                return std::make_pair(
                    websockets::State::OPEN,
                    FramePtr(
                        new Frame(Frame::Bit::FIN | Frame::Opcode::CONNECTION_CLOSE_FRAME,
                            payload.as_char, size)));
            }

            BOOST_FAIL("Improper state transition detected");
            return std::make_pair(websockets::State::ERROR, nullptr);
        }

        virtual websockets::State send_frame_impl(const Buffer& frame,
            websockets::Frame::Flags) override
        {
            Buffer input(frame);
            input.push_back(0);
            input.push_back(0);

            auto parser_retvals = parser::execute(input.data(), input.size());
            const parser::MessageList& messages = parser_retvals.first;
            const parser::ErrorList& errors = parser_retvals.second;

            BOOST_REQUIRE(errors.empty());

            std::for_each(messages.cbegin(), messages.cend(),
                [this](const Message& msg) {
                    client::State old_state = this->state_;
                    client::State new_state = client::transition(old_state, msg, Sender::CLIENT);

                    this->state_ = new_state;
                });

            return websockets::State::OPEN;
        }

        client::State state_;
        bool do_redirect_;
        std::string uri_;
    };

    constexpr const char RedirectionClient::DEFAULT_URI[];
    constexpr const char RedirectionClient::REDIRECTION_URI[];

    BOOST_AUTO_TEST_CASE(redirections)
    {
        std::unique_ptr<Client> p = Client::make(std::unique_ptr<websockets::Client>(new RedirectionClient),
            std::unique_ptr<ErrorHandler>(new FailHandler));

        p->login("auth", nullptr);

        BOOST_CHECK_EQUAL(p->getConnectionState(), client::State::CONNECTED);
        BOOST_CHECK_EQUAL(p->p_websocket_->uri(), RedirectionClient::REDIRECTION_URI);
    }
}
}
