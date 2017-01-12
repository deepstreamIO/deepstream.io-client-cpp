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

#include <ostream>

#include <client.hpp>
#include <message.hpp>

#include <cassert>


namespace deepstream {
namespace client
{

std::ostream& operator<<(std::ostream& os, State state)
{
	os << static_cast<int>(state);
	return os;
}



State transition(State state, const Message& message, Sender sender)
{
	assert( state != State::ERROR );
	assert( state != State::DISCONNECTED );

	const Topic topic = message.topic();
	const Action action = message.action();
	const bool is_ack = message.is_ack();
	const auto& arguments = message.arguments();


	if( state == State::AWAIT_CONNECTION &&
		topic == Topic::CONNECTION &&
		action == Action::CHALLENGE &&
		sender == Sender::SERVER )
	{
		assert( !is_ack );
		assert( arguments.empty() );

		return State::CHALLENGING;
	}

	if( state == State::CHALLENGING &&
		topic == Topic::CONNECTION &&
		action == Action::CHALLENGE_RESPONSE &&
		sender == Sender::CLIENT )
	{
		assert( !is_ack );
		assert( arguments.size() == 1 );

		return State::CHALLENGING_WAIT;
	}

	if( state == State::CHALLENGING_WAIT &&
		topic == Topic::CONNECTION &&
		action == Action::CHALLENGE_RESPONSE &&
		sender == Sender::SERVER )
	{
		assert( is_ack );
		assert( arguments.empty() );

		return State::AWAIT_AUTHENTICATION;
	}

	if( state == State::CHALLENGING_WAIT &&
		topic == Topic::CONNECTION &&
		action == Action::REJECT &&
		sender == Sender::SERVER )
	{
		assert( !is_ack );
		assert( arguments.empty() || arguments.size() == 1 );

		return State::DISCONNECTED;
	}

	if( state == State::AWAIT_AUTHENTICATION &&
		topic == Topic::AUTH &&
		action == Action::REQUEST &&
		sender == Sender::CLIENT )
	{
		assert( !is_ack );
		assert( arguments.size() == 2 );

		return State::AUTHENTICATING;
	}

	if( state == State::AUTHENTICATING &&
		topic == Topic::AUTH &&
		action == Action::REQUEST &&
		sender == Sender::SERVER )
	{
		assert( is_ack );
		assert( arguments.empty() );

		return State::CONNECTED;
	}

	if( state == State::AUTHENTICATING &&
		topic == Topic::AUTH &&
		action == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS &&
		sender == Sender::SERVER )
	{
		assert( !is_ack );
		assert( arguments.empty() );

		return State::DISCONNECTED;
	}

	if( state == State::AUTHENTICATING &&
		topic == Topic::AUTH &&
		action == Action::ERROR_INVALID_AUTH_DATA &&
		sender == Sender::SERVER )
	{
		assert( !is_ack );
		assert( arguments.empty() );

		return State::AWAIT_AUTHENTICATION;
	}

	return State::ERROR;
}

}
}
