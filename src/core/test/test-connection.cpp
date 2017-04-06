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
#include <iostream>

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

    std::ostream &operator<< (std::ostream &os, WSState state)
    {
        const char* states[] = {
            "ERROR",
            "OPEN",
            "CLOSED"
        };
        os << states[static_cast<int>(state)];
        return os;
    }

    struct FailHandler : public ErrorHandler {
        virtual void on_error(const std::string &) const override
        {
            BOOST_FAIL("There should be no errors");
        }
    };

    struct EventMock : public Event {
         EventMock(const SendFn &send_fn) : Event(send_fn)
         {
         }
    };

    struct PresenceMock : public Presence {
         PresenceMock(const SendFn &send_fn) : Presence(send_fn)
         {
         }
    };

    struct SimpleWSHandler : public WSHandler {
        SimpleWSHandler()
            : WSHandler()
        {
        }

        ~SimpleWSHandler()
        {
        }

        void process_messages()
        {
        }

        std::string URI() const override
        {
            return uri_;
        }

        void URI(std::string uri) override {
            uri_ = uri;
        }

        bool send(const Buffer &message) override
        {
            if (message == Message::from_human_readable("C|CHR+")) {
                const Buffer input = Message::from_human_readable("C|A|CHR+");
                (*on_message_)(std::move(input));
            } else if (message == Message::from_human_readable("C|CHR|ws://uri+")) {
                const Buffer input = Message::from_human_readable("C|A+");
                (*on_message_)(std::move(input));
            } else if (message == Message::from_human_readable("A|REQ|auth+")) {
                const Buffer input = Message::from_human_readable("A|A+");
                (*on_message_)(std::move(input));
            } else {
                std::cout << "unknown msg: " << std::string(message.cbegin(), message.cend()) << std::endl;
                assert(false);
            }

            return true;
        }

        void open() override {
            (*on_open_)();
            const Buffer input = Message::from_human_readable("C|CH+");
            (*on_message_)(std::move(input));
        }

        void close() override {}

        void reconnect() override {}

        void shutdown() override {}

        std::string uri_;
    };

    BOOST_AUTO_TEST_CASE(simple)
    {
        SimpleWSHandler wsh;
        FailHandler errh;
        EventMock evt([](const Message &){ return true; });
        PresenceMock pres([](const Message &){ return true; });
        Connection conn("ws://uri", wsh, errh, evt, pres);

        conn.login("auth", [](const std::unique_ptr<Buffer> &){});

        BOOST_CHECK_EQUAL(conn.state(), ConnectionState::OPEN);
    }

    struct RedirectionWSHandler : public WSHandler {
        RedirectionWSHandler() : WSHandler()
        {
        }

        ~RedirectionWSHandler()
        {
        }

        void process_messages() {}

        std::string URI() const override {
            return uri_;
        }

        void URI(std::string uri) override {
            uri_ = uri;
        }

        bool send(const Buffer &message) override
        {
            if (message == Message::from_human_readable("C|CHR+")) {
                const Buffer input = Message::from_human_readable("C|A|CHR+");
                (*on_message_)(std::move(input));
            } else if (message == Message::from_human_readable("C|CHR|ws://initial.uri+")) {
                const Buffer input = Message::from_human_readable("C|RED|ws://redirection.uri+");
                (*on_message_)(std::move(input));
            } else {
                std::cout << "unknown msg: " << std::string(message.cbegin(), message.cend()) << std::endl;
                assert(false);
            }
            return true;
        }

        void open() override {
            (*on_open_)();
            const Buffer input = Message::from_human_readable("C|CH+");
            (*on_message_)(std::move(input));
        }

        void close() override {}

        void reconnect() override {}

        void shutdown() override {}

        std::string uri_;
    };

    /*
     *constexpr const char RedirectionClient::DEFAULT_URI[];
     *constexpr const char RedirectionClient::REDIRECTION_URI[];
     */

    BOOST_AUTO_TEST_CASE(redirections)
    {
        RedirectionWSHandler wsh;
        FailHandler errh;
        EventMock evt([](const Message &){ return true; });
        PresenceMock pres([](const Message &){ return true; });
        Connection conn("ws://initial.uri", wsh, errh, evt, pres);

        conn.login("auth", [](const std::unique_ptr<Buffer> &){});

        BOOST_CHECK_EQUAL(conn.state(), ConnectionState::AWAIT_CONNECTION);
        BOOST_CHECK_EQUAL(wsh.URI(), "ws://redirection.uri");
    }

    BOOST_AUTO_TEST_CASE(lifetime)
    {
        auto make_msg = [](Topic topic, Action action) {
            return MessageBuilder(topic, action);
        };
        auto make_ack_msg = [](Topic topic, Action action) {
            return MessageBuilder(topic, action, true);
        };

        ConnectionState s0 = ConnectionState::AWAIT_CONNECTION;

        auto msg0 = make_msg(Topic::CONNECTION, Action::CHALLENGE);
        ConnectionState s1 = transition_incoming(s0, msg0);
        BOOST_CHECK_EQUAL(s1, ConnectionState::CHALLENGING);

        auto msg1 = make_msg(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
        msg1.add_argument(Buffer("URL"));
        ConnectionState s2 = transition_outgoing(s1, msg1);
        BOOST_CHECK_EQUAL(s2, ConnectionState::CHALLENGING_WAIT);

        auto msg2 = make_ack_msg(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
        ConnectionState s3 = transition_incoming(s2, msg2);
        BOOST_CHECK_EQUAL(s3, ConnectionState::AWAIT_AUTHENTICATION);

        auto msg3 = make_msg(Topic::AUTH, Action::REQUEST);
        msg3.add_argument(Buffer("{\"username\":\"u\",\"password\":\"p\"}"));
        ConnectionState s4 = transition_outgoing(s3, msg3);
        BOOST_CHECK_EQUAL(s4, ConnectionState::AUTHENTICATING);

        auto msg4 = make_ack_msg(Topic::AUTH, Action::REQUEST);
        ConnectionState s5 = transition_incoming(s4, msg4);
        BOOST_CHECK_EQUAL(s5, ConnectionState::OPEN);
    }
}
