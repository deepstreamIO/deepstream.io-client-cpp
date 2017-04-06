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

#pragma once

#include <functional>
#include <string>
#include <deepstream/core/ws.hpp>

#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/WebSocket.h>

namespace deepstream {

    class PocoWSHandler : public WSHandler {
    public:

        explicit PocoWSHandler();
        virtual ~PocoWSHandler();

        void process_messages();

        std::string URI() const override;

        void URI(std::string URI) override;

        /*
         * Send a message
         * returns true if sending was successful, false otherwise
         */
        bool send(const Buffer&) override;

        void open() override;

        void close() override;

        void reconnect() override;

        void shutdown() override;

    private:
        /*
         * Read a websocket frame into a buffer at the given byte offset
         * returns the number of bytes read.
         * Will not overflow buffer, but may lead to error state if there is
         * insufficient space in buffer for the frame to be read.
         */
        int read_frame(Buffer &, int offset);

        bool next_read_non_blocking();

        void state(const WSState);

        Poco::URI uri_;
        std::unique_ptr<Poco::Net::HTTPClientSession> session_;
        std::unique_ptr<Poco::Net::HTTPRequest> request_;
        std::unique_ptr<Poco::Net::HTTPResponse> response_;
        std::unique_ptr<Poco::Net::WebSocket> websocket_;

    };
}
