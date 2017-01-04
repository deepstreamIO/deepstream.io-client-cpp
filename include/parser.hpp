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
#ifndef DEEPSTREAM_PARSER_HPP
#define DEEPSTREAM_PARSER_HPP

#include <cstddef>

#include <message.hpp>


namespace deepstream
{
	namespace parser
	{
		struct Error
		{
			enum Tag
			{
				UNEXPECTED_TOKEN,
				UNEXPECTED_EOF,
				EMPTY_PAYLOAD
			};


			explicit Error(std::size_t offset, std::size_t length, Tag tag) :
				location_(offset, length),
				tag_(tag)
			{}

			Location location_;
			Tag tag_;
		};


		typedef deepstream_parser_state State;
	}
}


struct deepstream_parser_state
{
	typedef std::vector<deepstream::Message> MessageList;
	typedef std::vector<deepstream::parser::Error> ErrorList;

	explicit deepstream_parser_state(const char* buf) :
		buffer(buf),
		tokenizing_header(true),
		offset(0)
	{}

	const char* const buffer;
	bool tokenizing_header;
	std::size_t offset;

	MessageList messages;
	ErrorList errors;
};

#endif
