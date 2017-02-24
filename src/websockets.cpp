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
#include <deepstream/buffer.hpp>
#include <deepstream/websockets.hpp>

#include <cassert>

namespace deepstream {
namespace websockets {

    Frame::Frame(Flags flags, const char* payload, std::size_t size)
        : flags_(flags)
        , payload_(payload, payload + size)
    {
        assert(payload);
    }

    std::unique_ptr<Client> Client::construct(const std::string& uri) const
    {
        assert(!uri.empty());

        auto p = construct_impl(uri);
        assert(p);

        return p;
    }

    std::string Client::uri() const
    {
        std::string ret = uri_impl();
        assert(!ret.empty());

        return ret;
    }

    void Client::set_receive_timeout(time::Duration t)
    {
        set_receive_timeout_impl(t);
    }

    time::Duration Client::get_receive_timeout()
    {
        return get_receive_timeout_impl();
    }

    std::pair<State, std::unique_ptr<Frame> > Client::receive_frame()
    {
        return receive_frame_impl();
    }

    State Client::send_frame(const Buffer& buffer)
    {
        Frame::Flags f = Frame::Bit::FIN | Frame::Opcode::TEXT_FRAME;
        return send_frame(buffer, f);
    }

    State Client::send_frame(const Buffer& buffer, Frame::Flags flags)
    {
        assert(!buffer.empty());

        return send_frame_impl(buffer, flags);
    }

    void Client::close() { close_impl(); }
}
}
