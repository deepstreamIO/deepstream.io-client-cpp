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

#include <message_proxy.hpp>
#include <parser.h>


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
				CORRUPT_MESSAGE,
				INVALID_NUMBER_OF_ARGUMENTS
			};


			explicit Error(std::size_t offset, std::size_t length, Tag tag);

			const Location& location() const { return location_; }
			Tag tag() const { return tag_; }

			Location location_;
			Tag tag_;
		};


		typedef deepstream_parser_state State;
	}
}


struct deepstream_parser_state
{
	typedef std::vector<deepstream::parser::MessageProxy> MessageList;
	typedef std::vector<deepstream::parser::Error> ErrorList;

	explicit deepstream_parser_state(const char* p, std::size_t sz);

	// the lexer modifies its input. thus, the lexer works with a copy of the
	// input so the argument text cannot be assumed to be a substring of buffer_
	// starting at offset_.
	int handle_token(
		deepstream_token, const char* text, std::size_t);
	void handle_error(
		deepstream_token, const char*, std::size_t);
	void handle_header(
		deepstream_token, const char*, std::size_t);
	void handle_payload(
		deepstream_token, const char*, std::size_t);
	void handle_message_separator(
		deepstream_token, const char*, std::size_t);


	const char* const buffer_;
	const std::size_t buffer_size_;

	bool tokenizing_header_;
	std::size_t offset_;

	MessageList messages_;
	ErrorList errors_;
};

#endif
