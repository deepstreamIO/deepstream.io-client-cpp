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
#include <algorithm>
#include <limits>

#include <buffer.hpp>
#include <message_builder.hpp>
#include <random.hpp>

#include <cassert>


namespace deepstream {
namespace random
{

const Message::Header& make_header(Engine* p_engine)
{
	assert( p_engine );

	auto headers = Message::Header::all();
	const Message::Header* first = headers.first;
	const Message::Header* last = headers.second;

	std::size_t num_headers = last - first;
	std::uniform_int_distribution<std::size_t> dist(0, num_headers-1);

	const Message::Header* p = first + dist(*p_engine);

	return *p;
}


Buffer make_argument(
	Engine* p_engine, std::size_t min_size, std::size_t max_size)
{
	assert( p_engine );
	assert( min_size <= max_size );

	std::uniform_int_distribution<std::size_t> dsize(min_size, max_size);
	std::size_t size = dsize(*p_engine);

	static_assert( sizeof(Buffer::value_type) ==  1, "" );
	typedef Buffer::value_type T;
	T a = std::numeric_limits<T>::min();
	T b = std::numeric_limits<T>::max() - 2;

	std::uniform_int_distribution<T> dist(a, b);
	auto f = [&dist, p_engine] () -> T {
		T c = dist(*p_engine);
		c = (c < ASCII_RECORD_SEPARATOR) ? c : (c+2);

		assert( c != ASCII_RECORD_SEPARATOR );
		assert( c != ASCII_UNIT_SEPARATOR );

		return c;
	};

	Buffer buffer(size);
	std::generate( buffer.begin(), buffer.end(), f );

	return buffer;
}



MessageBuilder make_message(Engine* p_engine)
{
	assert( p_engine );

	const Message::Header& header = make_header(p_engine);

	return make_message(p_engine, header);
}


MessageBuilder make_message(Engine* p_engine, const Message::Header& header)
{
	assert( p_engine );

	auto expected_num_arguments = Message::num_arguments(header);
	std::size_t min_num_args = expected_num_arguments.first;
	std::size_t max_num_args = expected_num_arguments.second;

	MessageBuilder builder(header);

	std::uniform_int_distribution<std::size_t> dist(min_num_args, max_num_args);

	std::size_t num_args = dist(*p_engine);
	for(std::size_t i = 0; i < num_args; ++i)
	{
		builder.add_argument( make_argument(p_engine) );
	}

	return builder;
}



}
}
