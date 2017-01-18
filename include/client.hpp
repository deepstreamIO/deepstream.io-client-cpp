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
#ifndef DEEPSTREAM_CLIENT_HPP
#define DEEPSTREAM_CLIENT_HPP

#include <functional>
#include <iosfwd>
#include <list>
#include <vector>

#include <time.hpp>


namespace deepstream
{
	enum class Topic;
	enum class Action;
	enum class Sender;
	struct Message;


	namespace client
	{
		enum class State
		{
			ERROR,
			AWAIT_CONNECTION,
			CHALLENGING,
			CHALLENGING_WAIT,
			AWAIT_AUTHENTICATION,
			AUTHENTICATING,
			CONNECTED,
			DISCONNECTED
		};

		enum class Error
		{
			NONE,
			INVALID_STATE_TRANSITION,
			TIMEOUT,
			UNKNOWN_REQUEST
		};

		std::ostream& operator<<(std::ostream&, State);


		State transition(State, const Message&, Sender);


		struct Request
		{
			typedef std::vector<char> Pattern;


			explicit Request(Topic, Action, const Pattern& pattern);

			Topic topic() const { return topic_; }
			Action action() const { return action_; }
			const Pattern& pattern() const { return pattern_; }


			const Topic topic_;
			const Action action_;
			const Pattern pattern_;
		};


		struct Timer;


		struct Client
		{
			typedef std::list<Request> RequestList;
			typedef RequestList::iterator Key;

			static_assert( sizeof(Key) <= sizeof(void*), "" );


			explicit Client(Timer*);
			virtual ~Client();

			State send(const Message&);
			State receive(const Message&);
			State notify(Key);

			State handle_error_(const Message&, Sender, Error);
			State handle_timeout_(Key);

			virtual State handle_error_impl_(const Message&, Sender, Error) = 0;
			virtual State handle_timeout_impl_(Key) = 0;


			State state_;
			RequestList requests_;

			Timer* const p_timer_;
		};


		struct Timer
		{
			typedef State Return;
			typedef Client::Key Argument;
			typedef std::function<Return(Argument)> Callback;

			virtual ~Timer() {}

			void set(time::Duration, Callback, const Argument&);
			void unset(const Argument&);

			virtual void set_impl_(time::Duration, Callback, const Argument&)=0;
			virtual void unset_impl_(const Argument&) = 0;
		};
	}
}

#endif
