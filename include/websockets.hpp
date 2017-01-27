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


namespace deepstream
{
	namespace websockets
	{
		/**
		 * This structure contains the WebSocket frame flags.
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
			struct Flag
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
		};


		enum class StatusCode
		{
			UNKNOWN,
			NONE,
			NORMAL_CLOSE = 1000,
			ABNORMAL_CLOSE = 1006
		};
	}
}

#endif
