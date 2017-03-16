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
#include <deepstream/core/buffer.hpp>
#include "../websockets.hpp"
#include "pseudo.hpp"

#include <cassert>

namespace deepstream {
namespace websockets {
    namespace pseudo {

        Client::Client()
            : timeout_(time::Duration::max())
        {
        }

        std::string Client::uri_impl() const { return "pseudo"; }

        time::Duration Client::get_receive_timeout_impl() { return timeout_; }

        void Client::set_receive_timeout_impl(time::Duration t) { timeout_ = t; }

        void Client::close_impl() {}
    }
}
}
