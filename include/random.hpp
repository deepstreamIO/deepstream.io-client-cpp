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
#ifndef DEEPSTREAM_RANDOM_HPP
#define DEEPSTREAM_RANDOM_HPP

#include <random>

#include <message.hpp>


namespace deepstream
{
	struct Buffer;
	struct MessageBuilder;

	namespace random
	{
		typedef std::mt19937_64 Engine;

		const Message::Header& make_header(Engine*);
		Buffer make_argument(
			Engine*, std::size_t min_size=1, std::size_t max_size=100);
		MessageBuilder make_message(Engine*);
		MessageBuilder make_message(Engine*, const Message::Header&);
	}
}

#endif
