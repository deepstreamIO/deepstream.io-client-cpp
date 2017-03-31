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

        void send(const Buffer&) override;

        void open() override;

        void close() override;

        void reconnect() override;

        void shutdown() override;

        void on_open(const HandlerFn&) override;

        void on_close(const HandlerFn&) override;

        void on_error(const HandlerWithMsgFn&) override;

        void on_message(const HandlerWithBufFn&) override;

        WSState state() const override;

    private:
        void state(const WSState);

        Poco::URI uri_;
        std::unique_ptr<Poco::Net::HTTPClientSession> session_;
        std::unique_ptr<Poco::Net::WebSocket> websocket_;
        WSState state_;

        std::unique_ptr<HandlerFn> on_open_;
        std::unique_ptr<HandlerFn> on_close_;
        std::unique_ptr<HandlerWithBufFn> on_message_;
        std::unique_ptr<HandlerWithMsgFn> on_error_;
    };
}
