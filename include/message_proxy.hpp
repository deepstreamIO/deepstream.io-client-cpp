/*
 * Copyright 017 deepstreamHub GmbH
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
#ifndef DEEPSTREAM_MESSAGE_PROXY_HPP
#define DEEPSTREAM_MESSAGE_PROXY_HPP

#include <cstddef>

#include <vector>

#include <message.hpp>


namespace deepstream {
	struct Buffer;

	namespace parser
	{
		struct Location
		{
			explicit Location(std::size_t offset, std::size_t size) :
				offset_(offset),
				size_(size)
			{}

			std::size_t offset() const { return offset_; }
			std::size_t size() const { return size_; }

			std::size_t offset_;
			std::size_t size_;
		};


		struct MessageProxy : public Message
		{
			typedef std::vector<Location> LocationList;


			explicit MessageProxy(
				const char* p, std::size_t offset,
				Topic topic, Action action, bool is_ack=false);

			explicit MessageProxy(
				const char* p, std::size_t offset, const Message::Header&
			);


			const char* base() const { return base_; }
			std::size_t offset() const { return offset_; }


			virtual std::size_t size_impl_() const;

			virtual const Header& header_impl_() const;
			virtual std::size_t num_arguments_impl_() const;
			virtual Buffer get_impl_(std::size_t) const;

			virtual Buffer to_binary_impl_() const;



			const char* const base_;
			const std::size_t offset_;
			std::size_t size_;

			Message::Header header_;
			LocationList arguments_;
		};
	}
}

#endif

