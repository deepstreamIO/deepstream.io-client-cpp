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
#include <cstdio>

#include <algorithm>
#include <stdexcept>

#include <buffer.hpp>
#include <message_builder.hpp>
#include <presence.hpp>

#include <cassert>


namespace deepstream
{

Presence::Presence(const SendFn& send) :
	send_(send)
{
	assert( send_ );
}


Presence::SubscribeFnPtr Presence::subscribe(const SubscribeFn& f)
{
	SubscribeFnPtr p_f( new SubscribeFn(f) );
	subscribe(p_f);

	return p_f;
}


void Presence::subscribe(const SubscribeFnPtr& p_f)
{
	SubscriberList::const_iterator ci =
		std::find( subscribers_.cbegin(), subscribers_.cend(), p_f );

	if( ci != subscribers_.cend() )
		return;

	subscribers_.push_back(p_f);

	if( subscribers_.size() > 1 )
		return;


	MessageBuilder us(Topic::PRESENCE, Action::SUBSCRIBE);
	send_(us);
}



void Presence::unsubscribe(const SubscribeFnPtr& p_f)
{
	SubscriberList::iterator it =
		std::find( subscribers_.begin(), subscribers_.end(), p_f );

	if( it == subscribers_.end() )
		return;

	subscribers_.erase(it);

	if( !subscribers_.empty() )
		return;


	MessageBuilder uus(Topic::PRESENCE, Action::UNSUBSCRIBE);
	send_(uus);
}



void Presence::notify_(const Message& message)
{
	assert( message.topic() == Topic::PRESENCE );

	if( message.is_ack() )
	{
		assert( message.action() == Action::SUBSCRIBE ||
				message.action() == Action::UNSUBSCRIBE );
		return;
	}

	assert( message.action() == Action::PRESENCE_JOIN ||
			message.action() == Action::PRESENCE_LEAVE );

	bool is_login = message.action() == Action::PRESENCE_JOIN;

	// copy the list because subscribers may unsubscribe in the range-based for
	// loop  below
	SubscriberList subscribers(subscribers_);

	for(const auto& p_f : subscribers)
	{
		auto f = *p_f;
		f(message[0], is_login);
	}
}

}
