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
#ifndef DEEPSTREAM_WEBSOCKETS_POCO_HPP
#define DEEPSTREAM_WEBSOCKETS_POCO_HPP

#include <string>
#include <utility>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/URI.h>

#include "../src/core/time.hpp"

namespace deepstream {
struct Buffer;

namespace websockets {
    enum class StatusCode;
    struct Frame;
    struct WebSocketClient;

    namespace poco {
        struct PocoClient : public ::deepstream::websockets::WebSocketClient {
            explicit PocoClient(const std::string& uri, std::unique_ptr<Poco::Net::HTTPClientSession> session);

            // Construct a client based on the URI scheme type. It
            // will use a HTTPS session if the scheme matches "wss".
            static PocoClient* makeClient(const std::string& uri);

            std::size_t num_bytes_available();

            virtual std::unique_ptr<websockets::WebSocketClient>
            construct_impl(const std::string&) const override;

            virtual std::string uri_impl() const override;

            virtual time::Duration get_receive_timeout_impl() override;

            virtual void set_receive_timeout_impl(time::Duration) override;

            virtual std::pair<State, std::unique_ptr<Frame> >
            receive_frame_impl() override;

            virtual State send_frame_impl(const Buffer& buffer, Frame::Flags) override;

            virtual void close_impl() override;

            Poco::URI uri_;
            Poco::Net::HTTPRequest request_;
            Poco::Net::HTTPResponse response_;
            Poco::Net::WebSocket websocket_;
        };
    }
}
}

#endif
