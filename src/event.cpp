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



void Event::subscribe(const Name& name, const SubscribeFn& f)
{
	subscribe( name, SubscribeFnRef(new SubscribeFn(f)) );
}


void Event::subscribe(const Name& name, const SubscribeFnRef& p_f)
{
	assert( p_f );

	if( name.empty() )
		throw std::invalid_argument( "Empty event subscription pattern" );


	auto ret = subscriber_map_.equal_range(name);
	SubscriberMap::iterator first = ret.first;
	SubscriberMap::iterator last = ret.second;

	// avoid inserting duplicates in this branch
	if( first != last )
	{
		SubscriberMap::value_type value(name, p_f);
		SubscriberMap::iterator it = std::find(	first, last, value );

		if( it == last )
			subscriber_map_.emplace_hint( it, value );

		return;
	}


	subscriber_map_.emplace_hint( first, name, p_f );

	MessageBuilder message(Topic::EVENT, Action::SUBSCRIBE);
	message.add_argument( name );

	p_client_->send(message);
}


void Event::unsubscribe(const Name& name)
{
	auto ret = subscriber_map_.equal_range(name);

	if( ret.first == ret.second )
		return;

	subscriber_map_.erase( ret.first, ret.second );

	MessageBuilder message(Topic::EVENT, Action::UNSUBSCRIBE);
	message.add_argument(name);
	p_client_->send(message);
}


void Event::unsubscribe(const Name& name, const SubscribeFnRef& p_f)
{
	auto ret = subscriber_map_.equal_range(name);
	SubscriberMap::iterator first = ret.first;
	SubscriberMap::iterator last = ret.second;

	if( first == last )
		return;


	SubscriberMap::iterator it = std::find_if(
		first,
		last,
		[name, p_f] (const SubscriberMap::value_type& kv) {
			assert( kv.first == name );
			return kv.second == p_f;
		}
	);

	if( std::next(first) == last )
		unsubscribe(name);

	if( it != last )
		subscriber_map_.erase(it);
}


void Event::listen(const std::string& pattern, const ListenFn& f)
{
	listen( pattern, ListenFnRef(new ListenFn(f)) );
}


void Event::listen(const std::string& pattern, const ListenFnRef& p_f)
{
	ListenerMap::iterator it = listener_map_.find(pattern);

	if( it != listener_map_.end() )
		return;

	listener_map_.emplace( std::make_pair(pattern, p_f) );

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
