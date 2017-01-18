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
#include <algorithm>
#include <numeric>
#include <stdexcept>

#include <message.hpp>
#include <message_builder.hpp>

#include <cassert>


namespace deepstream
{

MessageBuilder::MessageBuilder(const Message::Header& header) :
	header_(header)
{
}



std::size_t MessageBuilder::size() const
{
	std::size_t size =
		header_.size() +
		arguments_.size() + // one separator for every argument
		std::accumulate(
			arguments_.cbegin(),
			arguments_.cend(),
			0,
			[] (std::size_t k, const Argument& arg) {return k + arg.size(); }
		) +
		1; // message separator

	return size;
}



std::vector<char> MessageBuilder::execute() const
{
	std::size_t size = this->size();
	std::vector<char> buffer(size);

	auto out = buffer.begin();

	const std::vector<char> bin_header = header_.to_binary();
	out = std::copy( bin_header.cbegin(), bin_header.cend(), out );

	for(auto in = arguments_.cbegin(); in != arguments_.cend(); ++in)
	{
		*out = ASCII_UNIT_SEPARATOR;
		++out;
		out = std::copy( in->cbegin(), in->cend(), out );
	}

	*out = ASCII_RECORD_SEPARATOR;
	assert( out+1 == buffer.end() );

	return buffer;
}



void MessageBuilder::add_argument(const Argument& arg)
{
	auto it = std::find(arg.cbegin(), arg.cend(), ASCII_UNIT_SEPARATOR);

	if( it != arg.cend() )
		throw std::invalid_argument("ASCII unit separator in payload detected");

	arguments_.push_back(arg);
}

}
