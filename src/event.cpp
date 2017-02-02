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
#include <stdexcept>
#include <iterator>

#include <buffer.hpp>
#include <deepstream.hpp>
#include <event.hpp>
#include <message_builder.hpp>

#include <cassert>


namespace deepstream
{


Event::Event(Client* p_client) :
	p_client_( p_client )
{
	assert( p_client );
}



void Event::subscribe(const Name& name, const SubscribeFnRef& p_f)
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
		assert(  !subscribers.empty() );

		SubscriberList::const_iterator it =
			std::find( subscribers.cbegin(), subscribers.cend(), p_f );

		if( it == subscribers.cend() )
			subscribers.push_back(p_f);

		return;
	}

	subscriber_map_[name].push_back(p_f);

	MessageBuilder message(Topic::EVENT, Action::SUBSCRIBE);
	message.add_argument( name );
	p_client_->send(message);
}


void Event::unsubscribe(const Name& name)
{
	SubscriberMap::iterator it = subscriber_map_.find(name);

	if( it == subscriber_map_.end() )
		return;

	subscriber_map_.erase(it);

	MessageBuilder message(Topic::EVENT, Action::UNSUBSCRIBE);
	message.add_argument(name);
	p_client_->send(message);
}


void Event::unsubscribe(const Name& name, const SubscribeFnRef& p_f)
{
	SubscriberMap::iterator it = subscriber_map_.find(name);

	if( it == subscriber_map_.end() )
		return;


	SubscriberList subscribers = it->second;
	SubscriberList::iterator ju =
		std::find( subscribers.begin(), subscribers.end(), p_f );

	if( ju == subscribers.end() )
		return;

	subscribers.erase(ju);

	if( subscribers.empty() )
		unsubscribe(name);
}



void Event::listen(const std::string& pattern, const ListenFnRef& p_f)
{
	ListenerMap::iterator it = listener_map_.find(pattern);

	if( it != listener_map_.end() )
		return;

	listener_map_[pattern] = p_f;

	MessageBuilder message(Topic::EVENT, Action::LISTEN);
	message.add_argument(pattern);
	p_client_->send(message);
}


void Event::unlisten(const std::string& pattern)
{
	ListenerMap::iterator it = listener_map_.find(pattern);

	if( it == listener_map_.end() )
		return;

	listener_map_.erase(it);

	MessageBuilder message(Topic::EVENT, Action::UNLISTEN);
	message.add_argument(pattern);
	p_client_->send(message);
}

}
