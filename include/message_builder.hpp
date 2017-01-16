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
#ifndef DEEPSTREAM_MESSAGE_BUILDER_HPP
#define DEEPSTREAM_MESSAGE_BUILDER_HPP

#include <vector>

#include <message.hpp>


namespace deepstream
{
	struct MessageBuilder
	{
		typedef std::vector<char> Argument;
		typedef std::vector<Argument> ArgumentList;


		explicit MessageBuilder(const Message::Header&);

		std::size_t size() const;
		std::vector<char> execute() const;

		void add_argument(const Argument& arg);

		const Message::Header& header() const { return header_; }
		const ArgumentList& arguments() const { return arguments_; }


		const Message::Header header_;
		ArgumentList arguments_;
	};
}

#endif
