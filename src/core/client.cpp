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

#include <cstdint>

#include <algorithm>
#include <chrono>
#include <stdexcept>

#include <ostream>

#include "connection.hpp"
#include "message.hpp"
#include "state.hpp"
#include "use.hpp"
#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>

#include <cassert>

namespace deepstream {

Client::Client(const std::string &uri, WSHandler &ws_handler, ErrorHandler &error_handler)
    : p_connection_(new Connection(uri, ws_handler, error_handler, event, presence))
    , event(std::bind(&Connection::send, p_connection_.get(), std::placeholders::_1))
    , presence(std::bind(&Connection::send, p_connection_.get(), std::placeholders::_1))
{
}

Client::~Client()
{
}

void Client::login(const Buffer& auth, const LoginCallback &callback)
{
    assert(p_connection_);
    p_connection_->login(auth, callback);
}

void Client::close() {
    return p_connection_->close();
}

ConnectionState Client::get_connection_state()
{
    return p_connection_->state();
}
}
