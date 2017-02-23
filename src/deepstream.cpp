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

#include <deepstream.hpp>
#include <deepstream/error_handler.hpp>
#include <deepstream/impl.hpp>
#include <deepstream/websockets.hpp>

#include <cassert>

namespace deepstream {

Client::Client(const std::string& uri, std::shared_ptr<ErrorHandler> p_eh)
    : p_impl_(impl::Client::make(uri, std::move(p_eh)).release())
    , event([this](const Message& message) -> bool {
        assert(p_impl_);
        return p_impl_->send_(message) == websockets::State::OPEN;
    })
    , presence([this](const Message& message) -> bool {
        assert(p_impl_);
        return p_impl_->send_(message) == websockets::State::OPEN;
    })
{
}

Client::Client(const std::string& uri)
    : Client(uri, std::shared_ptr<ErrorHandler>(new ErrorHandler()))
{
}

Client::~Client() { delete p_impl_; }

bool Client::login() { return p_impl_->login("{}"); }

bool Client::login(const std::string& auth, Buffer* p_user_data)
{
    return p_impl_->login(auth, p_user_data);
}

void Client::close() { return p_impl_->close(); }

client::State Client::getConnectionState()
{
    return p_impl_->getConnectionState();
}

void Client::process_messages()
{
    return p_impl_->process_messages(&event, &presence);
}
}
