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

#include <client.hpp>
#include <message.hpp>


namespace deepstream {
namespace client
{

BOOST_AUTO_TEST_CASE(lifetime)
{
	char dummy = 0;

	auto make_msg = [&dummy] (Topic topic, Action action) {
		return Message(&dummy, 0, topic, action);
	};
	auto make_ack_msg = [&dummy] (Topic topic, Action action) {
		return Message(&dummy, 0, topic, action, true);
	};

	State s0 = State::AWAIT_CONNECTION;

	Message msg0 = make_msg(Topic::CONNECTION, Action::CHALLENGE);
	State s1 = transition(s0, msg0, Sender::SERVER);
	BOOST_CHECK_EQUAL( s1, State::CHALLENGING );

	Message msg1 = make_msg(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
	msg1.arguments_.push_back( Location(1, 2) );
	State s2 = transition(s1, msg1, Sender::CLIENT );
	BOOST_CHECK_EQUAL( s2, State::CHALLENGING_WAIT );

	Message msg2 = make_ack_msg(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
	State s3 = transition(s2, msg2, Sender::SERVER);
	BOOST_CHECK_EQUAL( s3, State::AWAIT_AUTHENTICATION );

	Message msg3 = make_msg(Topic::AUTH, Action::REQUEST);
	msg3.arguments_.push_back( Location(3, 1) );
	msg3.arguments_.push_back( Location(4, 1) );
	State s4 = transition(s3, msg3, Sender::CLIENT);
	BOOST_CHECK_EQUAL( s4, State::AUTHENTICATING );

	Message msg4 = make_ack_msg(Topic::AUTH, Action::REQUEST);
	State s5 = transition(s4, msg4, Sender::SERVER);
	BOOST_CHECK_EQUAL( s5, State::CONNECTED );
}

}
}
