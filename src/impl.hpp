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
#ifndef DEEPSTREAM_IMPL_HPP
#define DEEPSTREAM_IMPL_HPP

#include <cstdint>

#include <functional>
#include <memory>
#include <string>

#include <deepstream/parser.hpp>

namespace deepstream {
struct Buffer;
struct ErrorHandler;
struct Event;
struct Message;
struct Presence;

namespace client {
    enum class State;
}

namespace websockets {
    enum class State;
    struct Client;
}

namespace impl {
    struct Client {
        static std::unique_ptr<Client>
        make(std::unique_ptr<websockets::Client> p_websocket,
            std::unique_ptr<ErrorHandler> p_error_handler);

        static std::unique_ptr<Client> make(const std::string& uri);

        static std::unique_ptr<Client> make(const std::string& uri,
            std::unique_ptr<ErrorHandler>);

        Client() = delete;

        Client(const Client&) = delete;

        Client(Client&&) = delete;

    protected:
        explicit Client(std::unique_ptr<websockets::Client> p_websocket,
            std::unique_ptr<ErrorHandler>);

    public:
        bool login();

        bool login(const std::string& auth, Buffer* p_user_data = nullptr);

        void close();

        client::State getConnectionState();

        void process_messages(Event*, Presence*);

        /**
	 * This function reads messages from the websocket.
	 *
	 * @param[out] p_buffer A non-NULL pointer. After a successful exit,
	 * the buffer stores the unparsed messages.
	 * @param[out] p_messages A non-NULL pointer. After a successful exit,
	 * the messages reference the storage of `p_buffer`.
	 */
        websockets::State receive_(Buffer* p_buffer, parser::MessageList* p_messages);

        /**
	 * This method serializes the given messages and sends it as a
	 * non-fragmented text frame to the server.
	 */
        websockets::State send_(const Message&);

        /**
	 * This method sends a non-fragmented text frame with the contents of
	 * the given buffer as payload.
	 */
        websockets::State send_frame_(const Buffer&);

        /**
	 * This method sends a frame with given flags and the contents of the
	 * given buffer as payload.
	 */
        websockets::State send_frame_(const Buffer&, int);

        client::State state_;
        std::unique_ptr<websockets::Client> p_websocket_;
        std::unique_ptr<ErrorHandler> p_error_handler_;
    };
}
}

#endif
