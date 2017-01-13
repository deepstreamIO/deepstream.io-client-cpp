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

#include <iosfwd>
#include <utility>
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
		CHALLENGE,
		CHALLENGE_RESPONSE,
		CREATE_OR_READ,
		ERROR_INVALID_AUTH_DATA,
		ERROR_TOO_MANY_AUTH_ATTEMPTS,
		LISTEN,
		QUERY,
		REDIRECT,
		REJECT,
		REQUEST,
		SUBSCRIBE,
		UNLISTEN,
		UNSUBSCRIBE
	};

	enum class Sender
	{
		CLIENT,
		SERVER
	};


	// needed by Boost.Test checks
	std::ostream& operator<<(std::ostream&, Topic);
	std::ostream& operator<<(std::ostream&, Action);
	std::ostream& operator<<(std::ostream&, Sender);


	struct Message
	{
		typedef std::vector<Location> LocationList;

		struct Header
		{
			static std::pair<const Header*, const Header*> all();

			static const char* to_string(Topic, Action, bool is_ack=false);
			static std::size_t size(Topic, Action, bool is_ack=false);

			/**
			 * This function takes a human-readable deepstream message, e.g.,
			 * `E|A|S|event+`, and returns in machine-readable counterpart by
			 * replacing `|` with the ASCII character 31 (unit separator) and
			 * `+` with ASCII character 30 (record separator).
			 *
			 * This functions returns vector<char> because vector<T>::data()
			 * returns sequential, writable memory while std::string::data()
			 * returns a pointer to const.
			 */
			static std::vector<char> from_human_readable(const char* p);
			static std::vector<char> from_human_readable(
				const char* p, std::size_t size);


			explicit Header(Topic topic, Action action, bool is_ack=false) :
				topic_(topic), action_(action), is_ack_(is_ack)
			{}


			const char* to_string() const;
			std::size_t size() const;

			Topic topic() const { return topic_; }
			Action action() const { return action_; }
			bool is_ack() const { return is_ack_; }


			Topic topic_;
			Action action_;
			bool is_ack_;
		};


		/**
		 * This function returns the minimum and maximum number of arguments for
		 * every message, e.g., for "E|S|event+" (event subscription), this
		 * function returns the pair (1, 1).
		 */
		static std::pair<std::size_t,std::size_t> num_arguments(
			Topic topic, Action action, bool is_ack);



		explicit Message(
			const char* p, std::size_t offset,
			Topic topic, Action action, bool is_ack=false);
		explicit Message(const char*, std::size_t, const Header&);


		const char* base() const { return base_; }
		std::size_t offset() const { return offset_; }
		std::size_t size() const { return size_; }

		const Header& header() const { return header_; }
		Topic topic() const { return header_.topic(); }
		Action action() const { return header_.action(); }
		bool is_ack() const { return header_.is_ack(); }

		const LocationList& arguments() const { return arguments_; }

		std::pair<std::size_t, std::size_t> num_arguments() const {
			return num_arguments( topic(), action(), is_ack() );
		}

		const char* const base_;
		const std::size_t offset_;
		std::size_t size_;

		Header header_;
		LocationList arguments_;
	};


	bool operator== (const Message::Header&, const Message::Header&);
}

#endif
