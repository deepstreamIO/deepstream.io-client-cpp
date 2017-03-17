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
#ifndef DEEPSTREAM_CLIENT_HPP
#define DEEPSTREAM_CLIENT_HPP

#include <deepstream/core/buffer.hpp>
#include <deepstream/core/event.hpp>
#include <deepstream/core/presence.hpp>

#include <cstdint>

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

namespace deepstream {
enum class Sender;
struct Message;

struct ErrorHandler;

namespace client {
    enum class State;
}

struct ClientImpl;

namespace client {
    /**
     * This enumeration stores the different client states of a deepstream
     * 2.x client.
     */
    enum class State {
        ERROR,
        AWAIT_CONNECTION,
        CHALLENGING,
        CHALLENGING_WAIT,
        AWAIT_AUTHENTICATION,
        AUTHENTICATING,
        CONNECTED,
        DISCONNECTED
    };
}

struct Client {
    Client(const std::string& uri);
    Client(const std::string& uri, std::unique_ptr<ErrorHandler>);

    ~Client();

    Client() = delete;

    Client(const Client&) = delete;

    Client(Client&&) = default;

public:
    /**
     * Given a client in `AWAIT_CONNECTION` state, this function attempts to
     * log in anonymously.
     *
     * @return `true` if the log in attempt was successful, `false`
     * otherwise
     */
    bool login();

    /**
     * Given a client in `AWAIT_CONNECTION` state, this function attempts to
     * log in with the given authentication data.
     *
     * The format of the user authentication data depends on the <a
     * href="https://deepstream.io/tutorials/core/security-overview/">authentication</a>
     * method used by the server.
     *
     * @param[in] auth User authentication data
     * @param[out] p_user_data On a successful reeturn, store the user data
     * in `p_user_data` if the reference is not `NULL`.
     *
     * @return `true` if the log in attempt was successful, `false`
     * otherwise
     */
    bool login(const std::string& auth, Buffer* p_user_data = nullptr);

    void close();

    client::State getConnectionState();

    /**
     * This function reads all incoming messages from the websocket and
     * executes the appropriate callbacks; it returns when there are no
     * messages left.
     */
    void process_messages();

    ClientImpl* const p_impl_;
    Event event;
    Presence presence;
};
}

#endif