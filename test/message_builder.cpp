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

#include <cstring>

#include <message_builder.hpp>


namespace deepstream
{

BOOST_AUTO_TEST_CASE(simple)
{
	Message::Header header(Topic::AUTH, Action::REQUEST, true);
	MessageBuilder builder(header);

	std::vector<char> bin_h = header.to_binary();
	std::vector<char> bin_b = builder.execute();

	BOOST_CHECK( std::equal(bin_h.cbegin(), bin_h.cend(), bin_b.cbegin()) );
}


BOOST_AUTO_TEST_CASE(message_with_payload)
{
	const char MSG[] = "A|REQ|user|password+";
	const char USER[] = "user";
	const char PASSWORD[] = "password";

	Message::Header header( Topic::AUTH, Action::REQUEST );
	MessageBuilder::Argument user( USER, USER + std::strlen(USER));
	MessageBuilder::Argument pwd( PASSWORD, PASSWORD + std::strlen(PASSWORD) );

	MessageBuilder builder(header);
	builder.add_argument(user);
	builder.add_argument(pwd);

	auto out = builder.execute();
	auto bin = Message::from_human_readable(MSG);

	BOOST_REQUIRE_EQUAL( out.size(), bin.size() );
	BOOST_CHECK( std::equal(bin.cbegin(), bin.cend(), out.cbegin()) );
}


BOOST_AUTO_TEST_CASE(binary_payload)
{
	typedef MessageBuilder::Argument Argument;

	const char MSG[] = "E|A|S|pattern|match+";

	Message::Header header(Topic::EVENT, Action::SUBSCRIBE);
	Argument arg1(1, 0);

	MessageBuilder builder(header);

	auto out = builder.execute();
	auto bin = Message::from_human_readable(MSG);

	BOOST_REQUIRE_EQUAL( out.size(), bin.size() );
}

}
