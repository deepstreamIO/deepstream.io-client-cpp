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
#include <utility>

#include <buffer.hpp>
#include <time.hpp>


namespace deepstream
{
	struct Buffer;

	namespace websockets
	{
		/**
		 * This structure stores as WebSockets frame.
		 *
		 * To have the WebSocket data match the values in these enums, the
		 * first eight bits of a frame must be interpreted in MSB order (most
		 * significant bit first, also known as network order). This agrees
		 * with the behavior of the POCO C++ WebSockets libraries.
		 *
		 * Enum classes are inappropriate here because a valid WebSocket frame
		 * always contains combinations of flags.
		 */
		struct Frame
		{
			typedef int Flags;

			struct Bit
			{
				enum
				{
					FIN =  1u << 7,
					RSV1 = 1u << 6,
					RSV2 = 1u << 5,
					RSV3 = 1u << 4
				};
			};

			struct Opcode
			{
				enum
				{
					CONTINUATION_FRAME = 0,
					TEXT_FRAME = 1,
					BINARY_FRAME = 2,
					CONNECTION_CLOSE_FRAME = 8,
					PING_FRAME = 9,
					PONG_FRAME = 10
				};
			};


			explicit Frame(Flags, const char*, std::size_t);

			Flags flags() const { return flags_; }
			const Buffer& payload() const { return payload_; }


			Flags flags_;
			Buffer payload_;
		};


		enum class StatusCode
		{
			UNKNOWN,
			NONE,
			NORMAL_CLOSE = 1000,
			ABNORMAL_CLOSE = 1006
		};


		enum class State
		{
			ERROR,
			OPEN,
			CLOSED
		};



		struct Client
		{
			virtual ~Client() {}


			std::string uri() const;

			std::size_t num_bytes_available();

			time::Duration get_receive_timeout();
			void set_receive_timeout(time::Duration);

			/*
			 * received no data: returns    (Status::OPEN, nullptr)
			 * receive data: returns        (Status::OPEN, frame)
			 * received close frame: return (Status::CLOSED, frame)
			 * received EOF: return         (Status::CLOSED, nullptr)
			 * received close frame with payload too long/too short:
			 *  return (Status::ERROR, frame)
			 */
			std::pair<State, std::unique_ptr<Frame> > receive_frame();

			/**
			 * @return 0 on connection close the number of bytes sent otherwise
			 */
			State send_frame(const Buffer&);
			State send_frame(const Buffer&, Frame::Flags);

			void close();


		protected:
			virtual std::string uri_impl() const = 0;

			virtual std::size_t num_bytes_available_impl() = 0;

			virtual time::Duration get_receive_timeout_impl() = 0;
			virtual void set_receive_timeout_impl(time::Duration) = 0;

			virtual std::pair<State, std::unique_ptr<Frame> >
				receive_frame_impl() = 0;
			virtual State send_frame_impl(const Buffer& buffer, Frame::Flags)=0;

			virtual void close_impl() = 0;
		};
	}
}

#endif
