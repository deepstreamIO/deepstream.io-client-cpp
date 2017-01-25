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

#include <vector>

#include <parser.h>
extern "C" {
#include <test_lexer.h>
}


struct State
{
	/**
	 * @param[in] p Pointer to scanner input (need not be null-terminated)
	 * @param[in] size Length of the string referenced by p
	 */
	explicit State(const char* p, std::size_t size) :
		input(size+2, 0)
	{
		std::memcpy(&input[0], p, size);

		for(std::size_t i = 0; i < size; ++i)
		{
			if(input[i] == '+') input[i] = '\x1e';
			if(input[i] == '|') input[i] = '\x1f';
		}

		int ret = yylex_init(&scanner);
		BOOST_REQUIRE_EQUAL(ret, 0);

		buffer = yy_scan_buffer(&input[0], size+2, scanner);
		yy_switch_to_buffer(buffer, scanner);
	}

	// convenience constructor for null-terminated strings
	explicit State(const char* p) :
		State(p, std::strlen(p))
	{}

	~State()
	{
		yy_delete_buffer(buffer, scanner);
		yylex_destroy(scanner);
	}


	std::vector<char> input;

	yyscan_t scanner;
	YY_BUFFER_STATE buffer;
};



BOOST_AUTO_TEST_CASE(empty_string)
{
	State state("");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );

	BOOST_CHECK_EQUAL( yyget_leng(state.scanner), 1 );
	BOOST_CHECK_EQUAL( *yyget_text(state.scanner), 0 );
}


BOOST_AUTO_TEST_CASE(auth_ack)
{
	State state("A|A+");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_A_A );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_MESSAGE_SEPARATOR );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


// Test detection of unknown tokens

BOOST_AUTO_TEST_CASE(unknown_token_header)
{
	State state("THIS_IS_AN_UNKNOWN_TOKEN");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_UNKNOWN );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


BOOST_AUTO_TEST_CASE(unknown_token_payload)
{
	State state("E|SERROR");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( yyget_leng(state.scanner), 3 );
	BOOST_CHECK_EQUAL( ret, TOKEN_E_S );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_UNKNOWN );
	BOOST_CHECK_EQUAL( "ERROR", yyget_text(state.scanner) );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


// Test error recovery

BOOST_AUTO_TEST_CASE(unknown_token_recovery)
{
	State state("THIS_IS_AN_UNKNOWN_TOKEN+A|A+");


	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_UNKNOWN );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_A_A );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_MESSAGE_SEPARATOR );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


BOOST_AUTO_TEST_CASE(unknown_token_recovery_MS_only)
{
	// Message separators \x1e are used for error recovery
	// Test what happens if the invalid message consists only of the single
	// token \x1e.

	State state("+A|A+");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_UNKNOWN );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_A_A );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_MESSAGE_SEPARATOR );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


// Test EOF is recognized in every state

BOOST_AUTO_TEST_CASE(recognize_EOF_state_header)
{
	State state("A|A");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_A_A );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


BOOST_AUTO_TEST_CASE(recognize_EOF_state_payload)
{
	State state("E|S|event");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_E_S );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_PAYLOAD );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


BOOST_AUTO_TEST_CASE(recognize_EOF_state_error)
{
	State state("ERROR");

	int ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_UNKNOWN );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
}


// Test sequence of messages

BOOST_AUTO_TEST_CASE(messages)
{
	State state("A|A+E|S|event+");
	deepstream_token tokens[] = {
		TOKEN_A_A,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_E_S,
		TOKEN_PAYLOAD,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_EOF
	};
	const std::size_t NUM_TOKENS = sizeof(tokens) / sizeof(tokens[0]);

	for(std::size_t i = 0; i < NUM_TOKENS; ++i)
	{
		int ret = yylex(state.scanner);
		BOOST_CHECK_EQUAL( ret, tokens[i] );
	}
}


//

BOOST_AUTO_TEST_CASE(nullchar)
{
	const char input[] = "E|S|eve\0nt+";
	std::size_t size = sizeof(input);

	State state(input, size);
	deepstream_token tokens[] = {
		TOKEN_E_S,
		TOKEN_PAYLOAD,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_EOF
	};
	const std::size_t NUM_TOKENS = sizeof(tokens) / sizeof(tokens[0]);

	for(std::size_t i = 0; i < NUM_TOKENS; ++i)
	{
		int ret = yylex(state.scanner);
		BOOST_CHECK_EQUAL( ret, tokens[i] );
	}
}


// test sequences of invalid messages
BOOST_AUTO_TEST_CASE(invalid_message_sequence)
{
	const char INPUT[] = "X|X|X++|+E|S|A|X+";

	const deepstream_token TOKENS[] = {
		TOKEN_UNKNOWN,
		TOKEN_UNKNOWN,
		TOKEN_E_S,
		TOKEN_PAYLOAD,
		TOKEN_PAYLOAD,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_EOF
	};
	const std::size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

	State state(INPUT);

	const std::size_t TEXTLENS[NUM_TOKENS] = { 7, 2, 3, 2, 2, 1, 1 };
	const char* const TEXTS[NUM_TOKENS] = {
		&state.input[ 0], &state.input[ 7], &state.input[ 9], &state.input[12],
		&state.input[14], &state.input[16], &state.input[17]
	};

	for(std::size_t i = 0; i < NUM_TOKENS; ++i)
	{
		int ret = yylex(state.scanner);
		BOOST_CHECK_EQUAL( ret, TOKENS[i] );

		std::size_t len = TEXTLENS[i];
		BOOST_REQUIRE_EQUAL( yyget_leng(state.scanner), len );
		BOOST_CHECK( !strncmp(yyget_text(state.scanner), TEXTS[i], len) );
	}
}


// test case with invalid message at the end of file and no message separator
BOOST_AUTO_TEST_CASE(invalid_message_at_eof)
{
	const char INPUT[] = "A|A+INVALID";

	const deepstream_token TOKENS[] = {
		TOKEN_A_A,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_UNKNOWN,
		TOKEN_EOF
	};
	const std::size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

	State state(INPUT);

	const std::size_t TEXTLENS[NUM_TOKENS] = { 3, 1, 7, 1 };
	const char* const TEXTS[NUM_TOKENS] = {
		&state.input[ 0], &state.input[ 3], &state.input[ 4], &state.input[11]
	};

	for(std::size_t i = 0; i < NUM_TOKENS; ++i)
	{
		int ret = yylex(state.scanner);
		BOOST_CHECK_EQUAL( ret, TOKENS[i] );

		std::size_t len = TEXTLENS[i];
		BOOST_REQUIRE_EQUAL( yyget_leng(state.scanner), len );
		BOOST_CHECK( !strncmp(yyget_text(state.scanner), TEXTS[i], len) );
	}
}


// regression test: carets must be allowed in the payload
BOOST_AUTO_TEST_CASE(caret_in_payload)
{
	const char INPUT[] = "E|S|a^b+";

	const deepstream_token TOKENS[] = {
		TOKEN_E_S,
		TOKEN_PAYLOAD,
		TOKEN_MESSAGE_SEPARATOR,
		TOKEN_EOF
	};
	const std::size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

	State state(INPUT);

	const std::size_t TEXTLENS[NUM_TOKENS] = { 3, 4, 1, 1 };
	const char* const TEXTS[NUM_TOKENS] = {
		&state.input[ 0], &state.input[ 3], &state.input[ 7], ""
	};

	for(std::size_t i = 0; i < NUM_TOKENS; ++i)
	{
		int ret = yylex(state.scanner);
		BOOST_CHECK_EQUAL( ret, TOKENS[i] );

		std::size_t len = TEXTLENS[i];
		BOOST_REQUIRE_EQUAL( yyget_leng(state.scanner), len );
		BOOST_CHECK( !strncmp(yyget_text(state.scanner), TEXTS[i], len) );
	}
}
