#ifndef DEEPSTREAM_PARSER_H
#define DEEPSTREAM_PARSER_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h> // EOF

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


#define \
	DS_PARSE(TOKEN) \
	deepstream_parser_handle( \
		yyget_extra(yyscanner), TOKEN, \
		yyget_text(yyscanner), yyget_leng(yyscanner) \
	); \


#if __cplusplus
}
#endif

#endif
