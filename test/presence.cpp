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
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <buffer.hpp>
#include <message.hpp>
#include <message_builder.hpp>
#include <presence.hpp>


namespace deepstream
{

BOOST_AUTO_TEST_CASE(simple)
{
	typedef Presence::Name Name;

	const Name name("name");

	bool is_subscribed = false;

	Presence::SendFn send = [&is_subscribed] (const Message& message) -> bool {
		BOOST_CHECK_EQUAL( message.topic(), Topic::PRESENCE );
		BOOST_CHECK( !message.is_ack() );

		if( message.action() == Action::SUBSCRIBE )
		{
			BOOST_CHECK( !is_subscribed );
			is_subscribed = true;
		}
		else if( message.action() == Action::UNSUBSCRIBE )
		{
			BOOST_CHECK( is_subscribed );
			is_subscribed = false;
		}
		else
			BOOST_FAIL( "This branch should not be taken" );

		return true;
	};


	unsigned num_calls = 0;
	Presence::SubscribeFn f =
		[name, &num_calls] (const Name& my_name, bool is_login) {
			BOOST_REQUIRE_EQUAL( name.size(), my_name.size() );
			BOOST_CHECK(
				std::equal( name.cbegin(), name.cend(), my_name.cbegin() )
			);
			BOOST_CHECK( !is_login );

			++num_calls;
		};

	Presence presence(send);

	const std::size_t N = 10;
	Presence::SubscriberList subscribers;
	for(std::size_t i = 0; i < N; ++i)
		subscribers.push_back( presence.subscribe(f) );

	BOOST_CHECK( is_subscribed );
	BOOST_CHECK_EQUAL( presence.subscribers_.size(), N );


	MessageBuilder pnl( Topic::PRESENCE, Action::PRESENCE_LEAVE );
	pnl.add_argument(name);
	presence.notify_(pnl);

	BOOST_CHECK( is_subscribed );
	BOOST_CHECK_EQUAL( num_calls, 10 );


	while( !subscribers.empty() )
	{
		Presence::SubscribeFnPtr p_f = subscribers.back();

		subscribers.pop_back();
		presence.unsubscribe(p_f);

		BOOST_CHECK_EQUAL( subscribers.size(), presence.subscribers_.size() );

	}

	BOOST_CHECK( !is_subscribed );
}

}
