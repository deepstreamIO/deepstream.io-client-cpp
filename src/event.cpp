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
#include <iterator>

#include <buffer.hpp>
#include <event.hpp>
#include <message_builder.hpp>

#include <cassert>


namespace deepstream
{


Event::Event(const SendFn& send) :
	send_(send)
{
	assert( send_ );
}



Event::SubscribeFnPtr Event::subscribe(const Name& name, const SubscribeFn& f)
{
	SubscribeFnPtr p_f( new SubscribeFn(f) );
	subscribe(name, p_f);

	return p_f;
}


void Event::subscribe(const Name& name, const SubscribeFnPtr& p_f)
{
	assert( p_f );

	if( name.empty() )
		throw std::invalid_argument( "Empty event subscription pattern" );


	auto ret = subscriber_map_.equal_range(name);
	SubscriberMap::iterator first = ret.first;
	SubscriberMap::iterator last = ret.second;

	if( first != last )
	{
		assert( first != subscriber_map_.end() );
		assert( first->first == name );
		assert( std::distance(first, last) == 1 );

		// avoid inserting duplicates
		SubscriberList& subscribers = first->second;
		assert( !subscribers.empty() );

		SubscriberList::const_iterator it =
			std::find( subscribers.cbegin(), subscribers.cend(), p_f );

		if( it == subscribers.cend() )
			subscribers.push_back(p_f);

		return;
	}

	subscriber_map_[name].push_back(p_f);

	MessageBuilder message(Topic::EVENT, Action::SUBSCRIBE);
	message.add_argument( name );
	send_(message);
}


void Event::unsubscribe(const Name& name)
{
	SubscriberMap::iterator it = subscriber_map_.find(name);

	if( it == subscriber_map_.end() )
		return;

	subscriber_map_.erase(it);

	MessageBuilder message(Topic::EVENT, Action::UNSUBSCRIBE);
	message.add_argument(name);
	send_(message);
}


void Event::unsubscribe(const Name& name, const SubscribeFnPtr& p_f)
{
	SubscriberMap::iterator it = subscriber_map_.find(name);

	if( it == subscriber_map_.end() )
		return;


	SubscriberList& subscribers = it->second;
	SubscriberList::iterator ju =
		std::find( subscribers.begin(), subscribers.end(), p_f );

	if( ju == subscribers.end() )
		return;

	subscribers.erase(ju);

	if( subscribers.empty() )
		unsubscribe(name);
}



void Event::listen(const std::string& pattern, const ListenFnPtr& p_f)
{
	ListenerMap::iterator it = listener_map_.find(pattern);

	if( it != listener_map_.end() )
		return;

	listener_map_[pattern] = p_f;

	MessageBuilder message(Topic::EVENT, Action::LISTEN);
	message.add_argument(pattern);
	send_(message);
}


void Event::unlisten(const std::string& pattern)
{
	ListenerMap::iterator it = listener_map_.find(pattern);

	if( it == listener_map_.end() )
		return;

	listener_map_.erase(it);

	MessageBuilder message(Topic::EVENT, Action::UNLISTEN);
	message.add_argument(pattern);
	send_(message);
}



void Event::notify_(const Message& message)
{
	assert( message.topic() == Topic::EVENT );

	switch( message.action() )
	{
		case Action::EVENT:
			assert( message.num_arguments() == 2 );
			notify_subscribers_(
				Name( message[0].cbegin(), message[0].cend() ),
				message[1]
			);
			break;

		default:
			assert(0);
	}
}


void Event::notify_subscribers_(const Name& name, const Buffer& data)
{
	SubscriberMap::iterator it = subscriber_map_.find(name);

	if( it == subscriber_map_.end() )
	{
		std::fprintf(
			stderr, "E|EVT: no subscriber named '%s'\n", name.c_str()
		);
		return;
	}

	assert( it->first == name );

	// Copying the list of subscribers is a necessity here because the callbacks
	// may unsubscribe during their execution and modifications of a subscriber
	// list may invalidate iterations and ranges. Also, copying the smart
	// pointer pointer here is a necessity because the referenced function may
	// decide to unsubscribe and in this case, the std::function object must not
	// be destructed. Copying ensures a non-zero reference count until `*p_f`
	// returns.
	// Finally, the iterator `it` may be invalid after executing a callback
	// because the list of subscribers for this `name` may have been erased.
	SubscriberList subscribers = it->second;
	it = subscriber_map_.end();

	for(const SubscribeFnPtr& p_f : subscribers)
	{
		auto f = *p_f;
		f(data);
	}
}

}
