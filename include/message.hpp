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


namespace deepstream
{
	struct Buffer;

	const char ASCII_RECORD_SEPARATOR = 30;
	const char ASCII_UNIT_SEPARATOR = 31;


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
		struct Header
		{
			static std::pair<const Header*, const Header*> all();

			static const char* to_string(Topic, Action, bool is_ack=false);
			static std::size_t size(Topic, Action, bool is_ack=false);

			explicit Header(Topic topic, Action action, bool is_ack=false) :
				topic_impl_(topic), action_impl_(action), is_ack_impl_(is_ack)
			{}


			const char* to_string() const;
			std::size_t size() const;

			Buffer to_binary() const;

			Topic topic() const { return topic_impl_; }
			Action action() const { return action_impl_; }
			bool is_ack() const { return is_ack_impl_; }


			Topic topic_impl_;
			Action action_impl_;
			bool is_ack_impl_;
		};



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
		static Buffer from_human_readable(const char* p);
		static Buffer from_human_readable(const char* p, std::size_t size);


		/**
		 * This function returns the minimum and maximum number of arguments for
		 * every message, e.g., for "E|S|event+" (event subscription), this
		 * function returns the pair (1, 1).
		 */
		static std::pair<std::size_t,std::size_t> num_arguments(const Header&);


		virtual ~Message() {}

		std::size_t size() const { return size_impl_(); }

		const Header& header() const { return header_impl_(); }
		Topic topic() const { return header().topic(); }
		Action action() const { return header().action(); }
		bool is_ack() const { return header().is_ack(); }

		std::size_t num_arguments() const { return num_arguments_impl_(); }
		Buffer operator[] (std::size_t) const;

		Buffer to_binary() const;


		virtual std::size_t size_impl_() const = 0;

		virtual const Header& header_impl_() const = 0;
		virtual std::size_t num_arguments_impl_() const = 0;
		virtual Buffer get_impl_(std::size_t) const = 0;

		virtual Buffer to_binary_impl_() const = 0;
	};


	std::ostream& operator<<(std::ostream&, const Message::Header&);

	bool operator== (const Message::Header&, const Message::Header&);
}

#endif
