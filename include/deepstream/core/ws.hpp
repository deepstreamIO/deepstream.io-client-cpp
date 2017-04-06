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
        typedef std::function<void(const std::string&&)> HandlerWithMsgFn;
        typedef std::function<void(const Buffer&&)> HandlerWithBufFn;
        typedef std::function<void()> HandlerFn;

        WSHandler()
            : on_open_(nullptr)
            , on_close_(nullptr)
            , on_message_(nullptr)
            , on_error_(nullptr)
            , state_(WSState::CLOSED)
        {}

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
        virtual bool send(const Buffer&) = 0;

        virtual void open() = 0;

        virtual void close() = 0;

        virtual void reconnect() = 0;

        virtual void shutdown() = 0;

        void on_open(const HandlerFn& on_open)
        {
            on_open_ = std::unique_ptr<HandlerFn>(new HandlerFn(on_open));
        }

        void on_close(const HandlerFn& on_close)
        {
            on_close_ = std::unique_ptr<HandlerFn>(new HandlerFn(on_close));
        }

        /*
         * Set the callback for handling messages. The callback shall be
         * invoked whenever messages are processed, with a buffer containing as
         * many whole messages as were in the socket buffer when checked. The
         * messages may be placed into the buffer consecutively, leaving the
         * parser responsible for delimiting messages.
         */
        void on_message(const HandlerWithBufFn& on_message)
        {
            on_message_ = std::unique_ptr<HandlerWithBufFn>(new HandlerWithBufFn(on_message));
        }

        void on_error(const HandlerWithMsgFn& on_error)
        {
            on_error_ = std::unique_ptr<HandlerWithMsgFn>(new HandlerWithMsgFn(on_error));
        }

        WSState state()
        {
            return state_;
        }

    protected:

        std::unique_ptr<HandlerFn> on_open_;
        std::unique_ptr<HandlerFn> on_close_;
        std::unique_ptr<HandlerWithBufFn> on_message_;
        std::unique_ptr<HandlerWithMsgFn> on_error_;

        WSState state_;
    };
}
