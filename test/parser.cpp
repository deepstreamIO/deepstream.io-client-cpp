/*
 * Copyright 2016-2017 deepstreamHub GmbH
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

#include <cstdio>
#include <cstring>

#include <numeric>
#include <vector>

#include <message.hpp>
#include <parser.hpp>


namespace deepstream {
namespace parser
{

BOOST_AUTO_TEST_CASE(empty_string)
{
	deepstream_parser_state state("", 0);

	int token = state.handle_token(TOKEN_EOF, "\0", 1);
	BOOST_CHECK_EQUAL( token, TOKEN_EOF );

	BOOST_CHECK( state.tokenizing_header_ );
	BOOST_CHECK_EQUAL( state.offset_, 1 );
	BOOST_CHECK( state.messages_.empty() );
	BOOST_CHECK( state.errors_.empty() );
}


BOOST_AUTO_TEST_CASE(simple)
{
	const auto input = Message::from_human_readable("A|A+", 5);
	const auto copy(input);
	const char* matches[] = { &input[0], &input[3], &input[4] };
	std::size_t sizes[] = { 3, 2, 1 };

	const deepstream_token tokens[] = {
		TOKEN_A_A, TOKEN_RECORD_SEPARATOR, TOKEN_EOF
	};

	const std::size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);


	deepstream_parser_state state( &copy[0], copy.size() );

	for(std::size_t i = 0; i < num_tokens; ++i)
	{
		int ret = state.handle_token(tokens[i], matches[i], sizes[i]);

		BOOST_CHECK_EQUAL( ret, tokens[i] );

		BOOST_CHECK_EQUAL( state.messages_.size(), 1 );
		BOOST_CHECK( state.errors_.empty() );

		std::size_t offset = std::accumulate( sizes, sizes+i+1, 0 );
		BOOST_CHECK_EQUAL( state.offset_, offset );
	}

	Message& msg = state.messages_.front();

	BOOST_CHECK_EQUAL( msg.base_, copy.data() );
	BOOST_CHECK_EQUAL( msg.offset_, 0 );
	BOOST_CHECK_EQUAL( msg.size_, 3 );

	BOOST_CHECK_EQUAL( msg.topic(), Topic::AUTH );
	BOOST_CHECK_EQUAL( msg.action(), Action::REQUEST );
	BOOST_CHECK( msg.is_ack() );

	BOOST_CHECK( msg.arguments_.empty() );
}

}
}
