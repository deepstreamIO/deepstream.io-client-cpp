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
#include <deepstream/core/buffer.hpp>

namespace deepstream {

    enum class WSState {
        ERROR,
        OPEN,
        CLOSED
    };

    class WSHandler {
    public:
        typedef std::function<void(const std::string&)> HandlerWithMsgFn;
        typedef std::function<void(const Buffer&)> HandlerWithBufFn;
        typedef std::function<void()> HandlerFn;

        WSHandler(){};
        WSHandler(WSHandler const&) = delete;
        WSHandler(WSHandler const&&) = delete;
        WSHandler& operator=(WSHandler const&) = delete;

        virtual ~WSHandler(){};

        virtual std::string URI() const = 0;
        virtual void URI(std::string URI) = 0;

        // Sends the contents of the given buffer through the socket as a
        // single frame.
        //
        // The frame should always be sent as FRAME_TEXT.
        virtual void send(const Buffer&) = 0;

        virtual void open() = 0;

        virtual void close() = 0;

        virtual void reconnect() = 0;

        virtual void shutdown() = 0;

        virtual void on_open(const HandlerFn&) = 0;

        virtual void on_close(const HandlerFn&) = 0;

        virtual void on_error(const HandlerWithMsgFn&) = 0;

        virtual void on_message(const HandlerWithBufFn&) = 0;

        virtual WSState state() const = 0;
    };
}
