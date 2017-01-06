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
#ifndef DEEPSTREAM_PARSER_H
#define DEEPSTREAM_PARSER_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#if __cplusplus
extern "C" {
#endif

struct deepstream_parser_state;


// The use of UCHAR_MAX below is motivated by the functions in, e.g., ctype.h
// which assume char values in the range 0-255.
enum deepstream_token
{
	TOKEN_EOF = EOF,
	TOKEN_UNKNOWN = UCHAR_MAX + 1,
	TOKEN_PAYLOAD,
	TOKEN_RECORD_SEPARATOR,
	TOKEN_A_A,
	TOKEN_A_E_IAD, // A|E|INVALID_AUTH_DATA
	TOKEN_A_E_TMAA, // A|E|TOO_MANY_AUTH_ATTEMPTS
	TOKEN_A_REQ,
	TOKEN_E_A_L,
	TOKEN_E_A_S,
	TOKEN_E_L,
	TOKEN_E_S,
	TOKEN_E_US
};


bool is_header_token(enum deepstream_token);


int deepstream_parser_handle(
	struct deepstream_parser_state*, enum deepstream_token,
	const char*, size_t);


#ifdef DEEPSTREAM_TEST_LEXER
#define DS_PARSE(TOKEN) (TOKEN)
#else
#define \
	DS_PARSE(TOKEN) \
	deepstream_parser_handle( \
		yyget_extra(yyscanner), TOKEN, \
		yyget_text(yyscanner), yyget_leng(yyscanner) \
	)
#endif


#if __cplusplus
}
#endif

#endif
