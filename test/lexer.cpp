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
	explicit State(const char* p) :
		input(strlen(p)+2, 0)
	{
		std::size_t size = std::strlen(p);
		std::strncpy(&input[0], p, size);

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
	BOOST_CHECK_EQUAL( ret, EOF );
}


BOOST_AUTO_TEST_CASE(unknown_token)
{
	State state("THIS_IS_AN_UNKNOWN_TOKEN");

	int ret = EOF;
	do
	{
		ret = yylex(state.scanner);
		printf("%d\n", ret);
	} while(ret != EOF);

	BOOST_CHECK_EQUAL( ret, TOKEN_UNKNOWN );

	ret = yylex(state.scanner);
	BOOST_CHECK_EQUAL( ret, TOKEN_EOF );
	BOOST_CHECK_EQUAL( ret, EOF );
}



