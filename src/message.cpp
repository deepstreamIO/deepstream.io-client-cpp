/*
 * Copyright 2016-2017 deepstreamHub GmbH
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

#include <limits>
#include <cstring>

#include <ostream>

#include <message.hpp>

#include <cassert>


namespace deepstream
{

std::ostream& operator<<(std::ostream& os, Topic topic)
{
	os << static_cast<int>(topic);
	return os;
}


std::ostream& operator<<(std::ostream& os, Action action)
{
	os << static_cast<int>(action);
	return os;
}



std::vector<char> Message::from_human_readable(const char* p)
{
	return Message::from_human_readable(p, std::strlen(p));
}

std::vector<char> Message::from_human_readable(const char* p, std::size_t size)
{
	std::vector<char> xs( size, 0 );
	std::memcpy( &xs[0], p, size );

	for(std::size_t i = 0; i < size; ++i)
	{
		if(xs[i] == '+') xs[i] = '\x1e';
		if(xs[i] == '|') xs[i] = '\x1f';
	}

	return xs;
}



std::pair<std::size_t, std::size_t> Message::num_arguments(
	Topic topic, Action action, bool is_ack)
{
	typedef std::pair<std::size_t, std::size_t> T;

	const std::size_t max = std::numeric_limits<std::size_t>::max();
	const T error(max, max);

	auto f = [] (std::size_t n) { return T(n, n); };

	switch(topic)
	{
		case Topic::AUTH:
			switch(action)
			{
				case Action::REQUEST:
					return is_ack ? f(0) : f(1);
				case Action::ERROR_INVALID_AUTH_DATA:
				case Action::ERROR_TOO_MANY_AUTH_ATTEMPTS:
					return f(0);

				default:
					assert(0);
					return error;
			}

		case Topic::CONNECTION:
			switch(action)
			{
				case Action::CHALLENGE:
					return f(0);

				case Action::CHALLENGE_RESPONSE:
					return is_ack ? f(0) : f(1);

				case Action::REDIRECT:
					return f(1);

				case Action::REJECT:
					return T(0, 1);

				default:
					assert(0);
					return error;
			}

		case Topic::EVENT:
			switch(action)
			{
				case Action::LISTEN:
				case Action::SUBSCRIBE:
				case Action::UNLISTEN:
				case Action::UNSUBSCRIBE:
					return is_ack ? f(2) : f(1);

				default:
					assert(0);
					return error;
			}

		default:
			assert(0);
	}

	return error;
}



Message::Message(
	const char* p, std::size_t offset, std::size_t header_size,
	Topic topic, Action action, bool is_ack) :
	base_(p),
	offset_(offset),
	size_(header_size),
	topic_(topic),
	action_(action),
	is_ack_(is_ack)
{
	assert(base_);
	assert(header_size > 0);
}

}
