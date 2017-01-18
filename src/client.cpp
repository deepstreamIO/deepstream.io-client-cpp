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
#include <algorithm>
#include <ostream>

#include <client.hpp>
#include <message.hpp>
#include <scope_guard.hpp>
#include <time.hpp>
#include <use.hpp>

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



Request::Request(Topic topic, Action action, const Pattern& pattern) :
	topic_(topic),
	action_(action),
	pattern_(pattern)
{
}



Client::Client(Timer* p_timer) :
	state_(State::AWAIT_CONNECTION),
	p_timer_(p_timer)
{
	assert( p_timer_ );
}


Client::~Client()
{
}


State Client::send(const Message& message)
{
	const Message::Header& header = message.header();
	const auto num_args = Message::num_arguments(header);
	use(num_args);

	assert(
		message.num_arguments() >= num_args.first &&
		message.num_arguments() <= num_args.second
	);

	State new_state = transition( state_, message, Sender::CLIENT );

	if( new_state == State::ERROR )
		return handle_error_(
			message, Sender::CLIENT, Error::INVALID_STATE_TRANSITION
		);


	DEEPSTREAM_ON_EXIT( [this, new_state] () {
		this->state_ = new_state;
	} );

	if( state_ != State::CONNECTED || new_state != State::CONNECTED )
		return new_state;


	Topic topic = header.topic();
	Action action = header.action();

	if( (topic == Topic::EVENT && action == Action::LISTEN) ||
		(topic == Topic::EVENT && action == Action::SUBSCRIBE) )
	{
		auto it = requests_.emplace(requests_.end(), topic, action, message[0]);

		time::Duration duration = time::Milliseconds(5000);
		Timer::Callback callback( [this] (Key key) {
			return this->notify(key);
		} );

		p_timer_->set( duration, callback, it );
	}

	return new_state;
}


State Client::receive(const Message& message)
{
	const Message::Header& header = message.header();
	const auto num_args = Message::num_arguments(header);
	use(num_args);

	assert(
		message.num_arguments() >= num_args.first &&
		message.num_arguments() <= num_args.second
	);

	State new_state = transition( state_, message, Sender::SERVER );

	if( new_state == State::ERROR )
		return handle_error_(
			message, Sender::SERVER, Error::INVALID_STATE_TRANSITION
		);


	if( state_ != State::CONNECTED || new_state != State::CONNECTED ||
		!header.is_ack() )
	{
		state_ = new_state;
		return new_state;
	}

	auto pattern = message[0];

	auto matches = [&header, &pattern] (const Request& req) { return
		header.topic() == req.topic() &&
		header.action() == req.action() &&
		std::equal(pattern.cbegin(), pattern.cend(), req.pattern().cbegin());
	};


	auto it = std::find_if( requests_.begin(), requests_.end(), matches );

	if( it == requests_.end() )
		return handle_error_(
			message, Sender::SERVER, Error::UNKNOWN_REQUEST
		);


	p_timer_->unset( it );
	state_ = new_state;

	return new_state;
}


State Client::notify(Key key)
{
	return handle_timeout_(key);
}


State Client::handle_error_(const Message& message, Sender sender, Error error)
{
	return handle_error_impl_(message, sender, error);
}


State Client::handle_timeout_(Key key)
{
	return handle_timeout_impl_(key);
}



void Timer::set(time::Duration duration, Callback f, const Argument& arg)
{
	assert( duration.count() > 0 );
	assert( f );

	set_impl_(duration, f, arg);
}


void Timer::unset(const Argument& arg)
{
	unset_impl_(arg);
}

}
}
