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
#ifndef DEEPSTREAM_CONNECTION_HPP
#define DEEPSTREAM_CONNECTION_HPP

#include <cstdint>

#include <functional>
#include <memory>
#include <string>

#include "parser.hpp"
#include "state.hpp"
#include <deepstream/core/client.hpp>

namespace deepstream {
    struct Buffer;
    struct ErrorHandler;
    struct Event;
    struct Message;
    struct Presence;

    struct Connection {

        Connection() = delete;

        Connection(const Connection&) = delete;

        Connection(Connection&&) = delete;

    public:

        explicit Connection(const std::string &, WSHandler &, ErrorHandler &);

        void login(const std::string& auth, const Client::LoginCallback &callback);

        void close();

        ConnectionState get_connection_state();

        /**
         * This method serializes the given message and sends it as a
         * non-fragmented text frame to the server.
         */
        void send_(const Message&);

    private:

        void on_message(const Buffer &message);
        void on_error(const std::string &error);
        void on_open();
        void on_close();

        ConnectionState state_;

        ErrorHandler &error_handler_;
        WSHandler &ws_handler_;

        std::unique_ptr<Client::LoginCallback> p_login_callback_;

    };
}

#endif
