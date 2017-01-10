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

#include <cstring>

#include <numeric>
#include <vector>

#include <message.hpp>
#include <parser.hpp>


// Remarks:
// - Do not use BOOST_CHECK_EQUAL() to compare const char* variables as
//   Boost.Test attempts to be smart by calling std::strcmp()
//   [with Boost 1.53, see boost/test/impl/test_tools.ipp:383, equal_impl()].
//   In this file, const char* variables do not always reference null-terminated
//   strings, e.g., when using std::vector<char*>::data(). If this happens,
//   Valgrind may detect invalid reads.


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
	const auto input = Message::from_human_readable("A|A+");
	const auto copy(input);
	const char* matches[] = { &input[0], &input[3], "" };
	std::size_t sizes[] = { 3, 1, 1 };

	const deepstream_token tokens[] = {
		TOKEN_A_A, TOKEN_MESSAGE_SEPARATOR, TOKEN_EOF
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

	BOOST_CHECK( msg.base_ == copy.data() );
	BOOST_CHECK_EQUAL( msg.offset_, 0 );
	BOOST_CHECK_EQUAL( msg.size_, input.size() );

	BOOST_CHECK_EQUAL( msg.topic(), Topic::AUTH );
	BOOST_CHECK_EQUAL( msg.action(), Action::REQUEST );
	BOOST_CHECK( msg.is_ack() );

	BOOST_CHECK( msg.arguments_.empty() );
}


BOOST_AUTO_TEST_CASE(concatenated_messages)
{
	const char STRING[] = "E|L|listen+E|S|event+";

	const auto input = Message::from_human_readable(STRING);
	const auto copy(input);

	const deepstream_token tokens[] = {
		TOKEN_E_L, TOKEN_PAYLOAD, TOKEN_MESSAGE_SEPARATOR,
		TOKEN_E_S, TOKEN_PAYLOAD, TOKEN_MESSAGE_SEPARATOR,
		TOKEN_EOF
	};

	const std::size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);

	const std::size_t matchlens[num_tokens] = { 3, 7, 1, 3, 6, 1, 1 };
	const char* matches[num_tokens] = {
		&input[ 0], &input[3], &input[10],
		&input[11], &input[14], &input[20],
		""
	};


	deepstream_parser_state state( &copy[0], copy.size() );

	for(std::size_t i = 0; i < num_tokens; ++i)
	{
		std::size_t offset = std::accumulate( matchlens, matchlens+i, 0 );
		BOOST_CHECK_EQUAL( state.offset_, offset );

		int ret = state.handle_token(tokens[i], matches[i], matchlens[i]);

		BOOST_CHECK_EQUAL( ret, tokens[i] );

		BOOST_CHECK_EQUAL( state.messages_.size(), (i>=3) ? 2 : 1 );
		BOOST_CHECK( state.errors_.empty() );

		BOOST_CHECK_EQUAL( state.offset_, offset+matchlens[i] );
	}

	for(const Message& msg : state.messages_)
	{
		BOOST_CHECK( msg.base_ == copy.data() );

		BOOST_CHECK_EQUAL( msg.topic(), Topic::EVENT );
		BOOST_CHECK( !msg.is_ack() );

		BOOST_CHECK_EQUAL( msg.arguments_.size(), 1 );
	}

	BOOST_CHECK_EQUAL( state.messages_.size(), 2 );

	const Message& msg_f = state.messages_.front();
	BOOST_CHECK_EQUAL( msg_f.offset(), 0 );
	BOOST_CHECK_EQUAL( msg_f.size(), 11 );
	BOOST_CHECK_EQUAL( msg_f.topic(), Topic::EVENT );
	BOOST_CHECK_EQUAL( msg_f.action(), Action::LISTEN );

	const Location& arg_f = msg_f.arguments_.front();

	BOOST_CHECK_EQUAL( arg_f.offset_, 4 );
	BOOST_CHECK_EQUAL( arg_f.length_, 6 );
	BOOST_CHECK( !strncmp(&input[arg_f.offset_], "listen", arg_f.length_) );


	const Message& msg_b = state.messages_.back();
	BOOST_CHECK_EQUAL( msg_b.offset(), 11 );
	BOOST_CHECK_EQUAL( msg_b.size(), 10 );
	BOOST_CHECK_EQUAL( msg_b.topic(), Topic::EVENT );
	BOOST_CHECK_EQUAL( msg_b.action(), Action::SUBSCRIBE );

	const Location& arg_b = msg_b.arguments_.front();

	BOOST_CHECK_EQUAL( arg_b.offset_, 15 );
	BOOST_CHECK_EQUAL( arg_b.length_, 5 );
	BOOST_CHECK( !strncmp(&input[arg_b.offset_], "event", arg_b.length_) );
}



BOOST_AUTO_TEST_CASE(invalid_number_of_arguments)
{
	const char STRING[] = "E|A|L|l+";

	const auto input = Message::from_human_readable(STRING);
	const auto copy(input);

	const deepstream_token TOKENS[] = {
		TOKEN_E_A_L, TOKEN_PAYLOAD, TOKEN_MESSAGE_SEPARATOR, TOKEN_EOF
	};

	const std::size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

	const std::size_t MATCHLENS[NUM_TOKENS] = { 5, 2, 1, 1 };
	const char* MATCHES[NUM_TOKENS] = { &input[0], &input[5], &input[7], "" };


	deepstream_parser_state state( &copy[0], copy.size() );

	for(std::size_t i = 0; i < NUM_TOKENS; ++i)
	{
		std::size_t offset = std::accumulate( MATCHLENS, MATCHLENS+i, 0 );
		BOOST_CHECK_EQUAL( state.offset_, offset );

		int ret = state.handle_token(TOKENS[i], MATCHES[i], MATCHLENS[i]);

		BOOST_CHECK_EQUAL( ret, TOKENS[i] );

		BOOST_CHECK_EQUAL( state.messages_.size(), (i>=2) ? 0 : 1 );
		BOOST_CHECK_EQUAL( state.errors_.size(), (i>=2) ? 1 : 0 );

		BOOST_CHECK_EQUAL( state.offset_, offset+MATCHLENS[i] );
	}


	const Error& e = state.errors_.front();

	BOOST_CHECK_EQUAL( e.location_.offset_, 0 );
	BOOST_CHECK_EQUAL( e.location_.length_, 8 );
	BOOST_CHECK_EQUAL( e.tag_, Error::INVALID_NUMBER_OF_ARGUMENTS );
}

}
}
