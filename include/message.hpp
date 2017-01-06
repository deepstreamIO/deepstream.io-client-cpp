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
#ifndef DEEPSTREAM_MESSAGE_HPP
#define DEEPSTREAM_MESSAGE_HPP


#include <cstddef>
#include <vector>


namespace deepstream
{
	struct Location
	{
		explicit Location(std::size_t offset, std::size_t length) :
			offset_(offset),
			length_(length)
		{}


		std::size_t offset_;
		std::size_t length_;
	};


	enum class Topic
	{
		AUTH,
		CONNECTION,
		ERROR,
		EVENT,
		RECORD,
		RPC
	};

	enum class Action
	{
		CREATE_OR_READ,
		ERROR_INVALID_AUTH_DATA,
		ERROR_TOO_MANY_AUTH_ATTEMPTS,
		LISTEN,
		QUERY,
		REQUEST,
		SUBSCRIBE,
		UNLISTEN,
		UNSUBSCRIBE
	};


	struct Message
	{
		typedef std::vector<Location> LocationList;

		explicit Message(
			const char* p, std::size_t offset, std::size_t header_size,
			Topic topic, Action action, bool isAck=false);

		const char* base() const { return base_; }
		std::size_t offset() const { return offset_; }
		std::size_t size() const { return size_; }

		Topic topic() const { return topic_; }
		Action action() const { return action_; }
		bool isAck() const { return isAck_; }

		const char* const base_;
		const std::size_t offset_;
		std::size_t size_;

		const Topic topic_;
		const Action action_;
		const bool isAck_;

		LocationList arguments_;
	};
}

#endif
