#ifndef DEEPSTREAM_PARSER_H
#define DEEPSTREAM_PARSER_H

#include <stdbool.h>
#include <stddef.h>

#if __cplusplus
extern "C" {
#endif

struct deepstream_parser_state;


enum deepstream_token
{
	TOKEN_EOF = EOF,
	TOKEN_UNKNOWN = 257,
	TOKEN_PAYLOAD,
	TOKEN_RECORD_SEPARATOR,
	TOKEN_A_A,
	TOKEN_A_REQ,
	TOKEN_E_A_S,
	TOKEN_E_L,
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


void* yyalloc (size_t bytes, void * yyscanner);
void* yyrealloc (void * ptr, size_t bytes, void * yyscanner);
void yyfree (void * ptr, void * yyscanner);


#if __cplusplus
}
#endif

#endif
