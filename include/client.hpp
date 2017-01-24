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

#include <iosfwd>


namespace deepstream
{
	enum class Sender;
	struct Message;

	namespace client
	{
		/**
		 * This enumeration stores the different client states of a deepstream
		 * 2.x client.
		 */
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

		std::ostream& operator<<(std::ostream&, State);


		/**
		 * Given current client state, a message, and a sender, this function
		 * returns the next state of the client's finite state machine.
		 */
		State transition(State s, const Message& message, Sender sender);
	}
}

#endif
