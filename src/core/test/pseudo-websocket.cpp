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
#include "pseudo-websocket.hpp"

namespace deepstream {

    PseudoWSHandler::PseudoWSHandler() : WSHandler()
    {}

    PseudoWSHandler::~PseudoWSHandler(){}

    void PseudoWSHandler::process_messages(){}

    std::string PseudoWSHandler::URI() const
    {
        return "";
    }

    void PseudoWSHandler::URI(std::string URI){}

    void PseudoWSHandler::send(const Buffer&){}

    void PseudoWSHandler::open(){}

    void PseudoWSHandler::close(){}

    void PseudoWSHandler::reconnect(){}

    void PseudoWSHandler::shutdown(){}

    void PseudoWSHandler::on_open(const HandlerFn&){}

    void PseudoWSHandler::on_close(const HandlerFn&){}

    void PseudoWSHandler::on_error(const HandlerWithMsgFn&){}

    void PseudoWSHandler::on_message(const HandlerWithBufFn&){}

    WSState PseudoWSHandler::state() const
    {
        return WSState::CLOSED;
    }

}
