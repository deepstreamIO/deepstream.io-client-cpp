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
#ifndef DEEPSTREAM_WEBSOCKETS_PSEUDO_HPP
#define DEEPSTREAM_WEBSOCKETS_PSEUDO_HPP

#include <memory>
#include <string>
#include <utility>

#include <deepstream/core/ws.hpp>


namespace deepstream {
    struct Buffer;

    struct PseudoWSHandler : WSHandler {

        explicit PseudoWSHandler();

        ~PseudoWSHandler();

        void process_messages();

        std::string URI() const override;

        void URI(std::string URI) override;

        bool send(const Buffer&) override;

        void open() override;

        void close() override;

        void reconnect() override;

        void shutdown() override;

        void on_open(const HandlerFn&) override;

        void on_close(const HandlerFn&) override;

        void on_error(const HandlerWithMsgFn&) override;

        void on_message(const HandlerWithBufFn&) override;

        WSState state() const override;

    };

}

#endif
