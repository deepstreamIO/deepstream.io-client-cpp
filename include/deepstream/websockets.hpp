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
#ifndef DEEPSTREAM_WEBSOCKETS_HPP
#define DEEPSTREAM_WEBSOCKETS_HPP

#include <memory>
#include <string>
#include <utility>

#include <deepstream/time.hpp>

namespace deepstream {
struct Buffer;

namespace websockets {
    /**
     * This structure stores a WebSockets frame.
     *
     * To have the WebSocket data match the values in these enums, the
     * first eight bits of a frame must be interpreted in MSB order (most
     * significant bit first, also known as network order). This agrees
     * with the behavior of the POCO C++ WebSockets libraries.
     *
     * Enum classes are inappropriate here because a valid WebSocket frame
     * always contains combinations of flags.
     */
    struct Frame {
        typedef int Flags;

        struct Bit {
            enum { FIN = 1u << 7,
                RSV1 = 1u << 6,
                RSV2 = 1u << 5,
                RSV3 = 1u << 4 };
        };

        struct Opcode {
            enum {
                CONTINUATION_FRAME = 0,
                TEXT_FRAME = 1,
                BINARY_FRAME = 2,
                CONNECTION_CLOSE_FRAME = 8,
                // pings are implemented in deepstream as `C|PI+` messages
                PING_FRAME = 9,
                // pongs are implemented in deepstream as `C|PO+` messages
                PONG_FRAME = 10
            };
        };

        explicit Frame(Flags, const char*, std::size_t);

        Flags flags() const { return flags_; }

        const Buffer& payload() const { return payload_; }

        Flags flags_;
        Buffer payload_;
    };

    enum class StatusCode {
        UNKNOWN,
        NONE,
        NORMAL_CLOSE = 1000,
        ABNORMAL_CLOSE = 1006
    };

    enum class State { ERROR,
        OPEN,
        CLOSED };

    struct Client {
        virtual ~Client() = default;

        /**
	 * This method returns a new object of the concrete class
	 * implementing this interface with the new object being connected
	 * to the given URI.
	 */
        std::shared_ptr<Client> construct(const std::string& uri) const;

        std::string uri() const;

        time::Duration get_receive_timeout();

        void set_receive_timeout(time::Duration);

        /**
	 * This function tries to receive a single websocket frame.
	 *
	 * The meaning of the various combinations of return values is
	 * listed below:
	 * - received a frame:         (Status::OPEN,   frame)
	 * - no data (socket timeout): (Status::OPEN,   nullptr)
	 * - received close frame:     (Status::CLOSED, frame)
	 * - received EOF:             (Status::CLOSED, nullptr)
	 * - received close frame with payload too long/too short:
	 *                             (Status::ERROR,  frame)
	 */
        std::pair<State, std::unique_ptr<Frame> > receive_frame();

        /**
	 * This function sends a non-fragmented text frame with the contents
	 * of the buffer as payload.
	 *
	 * @return 0 on connection close or the number of bytes sent
	 * otherwise
	 */
        State send_frame(const Buffer&);

        /**
	 * This function sends a frame with the given flags and with the
	 * contents of the buffer as payload.
	 *
	 * @return 0 on connection close or the number of bytes sent
	 * otherwise
	 */
        State send_frame(const Buffer&, Frame::Flags);

        void close();

    protected:
        virtual std::shared_ptr<websockets::Client>
        construct_impl(const std::string&) const = 0;

        virtual std::string uri_impl() const = 0;

        virtual time::Duration get_receive_timeout_impl() = 0;

        virtual void set_receive_timeout_impl(time::Duration) = 0;

        virtual std::pair<State, std::unique_ptr<Frame> > receive_frame_impl() = 0;

        virtual State send_frame_impl(const Buffer& buffer, Frame::Flags) = 0;

        virtual void close_impl() = 0;
    };
}
}

#endif
