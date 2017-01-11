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
#include <random>
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
		bool tokenizing_header = state.tokenizing_header_;
		BOOST_CHECK( (i==0||i==2) ? tokenizing_header : !tokenizing_header );

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
		BOOST_CHECK( (i==0||i==3||i==6) ?
			state.tokenizing_header_ :
			!state.tokenizing_header_
		);

		std::size_t offset = std::accumulate( matchlens, matchlens+i, 0 );
		BOOST_CHECK_EQUAL( state.offset_, offset );

		int ret = state.handle_token(tokens[i], matches[i], matchlens[i]);

		BOOST_CHECK_EQUAL( ret, tokens[i] );

		BOOST_CHECK_EQUAL( state.messages_.size(), (i>=3) ? 2 : 1 );
		BOOST_CHECK( state.errors_.empty() );

		BOOST_CHECK_EQUAL( state.offset_, offset+matchlens[i] );
	}

	BOOST_CHECK( state.tokenizing_header_ );

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
		bool tokenizing_header = state.tokenizing_header_;
		BOOST_CHECK( (i==0||i==3) ? tokenizing_header : !tokenizing_header );

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



BOOST_AUTO_TEST_CASE(random_tokens)
{
	const deepstream_token TOKENS[] = {
		TOKEN_UNKNOWN,
		TOKEN_PAYLOAD,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_A_A,
		TOKEN_A_E_IAD,
		TOKEN_A_E_TMAA,
		TOKEN_A_REQ,
		TOKEN_E_A_L,
		TOKEN_E_A_S,
		TOKEN_E_L,
		TOKEN_E_S,
		TOKEN_E_US
	};

	const std::size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);
	const std::size_t N = 1000;

	for(std::size_t iteration = 0; iteration < 10; ++iteration)
	{
		std::mt19937_64 engine( iteration );

		std::uniform_int_distribution<std::size_t> rand3(0, 2);
		std::uniform_int_distribution<std::size_t> rand_index(0, NUM_TOKENS-1);
		std::uniform_int_distribution<std::size_t> rand_uint(1, 20);
		std::uniform_int_distribution<char> rand_char;

		auto generate3 =
			[&engine, &rand3] () { return rand3(engine); };
		auto generate_index =
			[&engine, &rand_index] () { return rand_index(engine); };
		auto generate_uint =
			[&engine, &rand_uint] () { return rand_uint(engine); };
		auto generate_char =
			[&engine, &rand_char] () { return rand_char(engine); };

		std::vector<char> input(N);
		std::generate( input.begin(), input.end(), generate_char );

		std::vector<char> copy(input);

		State state( copy.data(), copy.size() );

		for(std::size_t offset = 0; offset < N; )
		{
			std::size_t index = state.tokenizing_header_
				? generate_index()
				: generate3();
			deepstream_token token = TOKENS[index];

			std::size_t textlen = (token == TOKEN_MESSAGE_SEPARATOR)
				? 1
				: std::min(generate_uint(), N-offset);

			if( state.messages_.empty() &&
				(token == TOKEN_MESSAGE_SEPARATOR || token == TOKEN_PAYLOAD) )
				token = TOKEN_UNKNOWN;

			if( token == TOKEN_PAYLOAD )
				copy[offset] = input[offset] = ASCII_UNIT_SEPARATOR;
			if( token == TOKEN_MESSAGE_SEPARATOR )
				copy[offset] = input[offset] = ASCII_RECORD_SEPARATOR;

			std::size_t num_messages = state.messages_.size();
			std::size_t num_errors = state.errors_.size();

			int ret = deepstream_parser_handle(
				&state, token, &input[offset], textlen
			);

			BOOST_CHECK_EQUAL( ret, token );

			if( token == TOKEN_UNKNOWN )
			{
				BOOST_CHECK( state.tokenizing_header_ );
				BOOST_CHECK(
					state.messages_.size() == num_messages + 1 ||
					state.errors_.size() == num_errors + 1
				);
			}
			if( token == TOKEN_MESSAGE_SEPARATOR )
				BOOST_CHECK( state.tokenizing_header_ );
			if( is_header_token(token) )
				BOOST_CHECK( !state.tokenizing_header_ );

			offset += textlen;

			BOOST_CHECK_EQUAL( offset, state.offset_ );
		}

		int ret = deepstream_parser_handle(&state, TOKEN_EOF, "", 1);

		BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
		BOOST_CHECK( state.tokenizing_header_ );
		BOOST_CHECK_EQUAL( state.offset_, N+1 );
	}
}

}
}
