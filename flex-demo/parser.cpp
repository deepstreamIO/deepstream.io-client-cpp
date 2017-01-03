#include <cstdio>

#include "parser.h"
#include "parser.hpp"

#include <cassert>



bool is_header_token(enum deepstream_token token)
{
	int min_event_num = TOKEN_A_A;

	return token >= min_event_num;
}



int deepstream_parser_handle(
	deepstream::parser::State* p_state, deepstream_token token,
	const char* match, std::size_t matchlen)
{
	assert( p_state );
	assert( match );
	assert( matchlen > 0 );

	std::size_t offset = p_state->offset;
	p_state->offset = offset + matchlen;

	std::printf("offset=%zu len=%zu\n", offset, matchlen);

	if(token == TOKEN_UNKNOWN)
	{
		fprintf(
			stderr,
			"Unknown token %zu bytes into the message\n",
			offset
		);

		return token;
	}

	if( p_state->tokenizing_header && token == TOKEN_EOF )
		return token;

	if( p_state->tokenizing_header && !is_header_token(token) )
	{
		fprintf(
			stderr,
			"Expected header %zu bytes into the message\n",
			offset
		);

		return token;
	}

	if( !p_state->tokenizing_header && is_header_token(token) )
	{
		fprintf(
			stderr,
			"Expected payload, found header %zu bytes into the message\n",
			offset
		);

		return token;
	}

	if( !p_state->tokenizing_header && token == TOKEN_EOF )
	{
		fprintf(
			stderr,
			"Unexpected EOF %zu bytes into the message\n",
			offset
		);

		return token;
	}


	// handle legal tokens
	if( p_state->tokenizing_header && is_header_token(token) )
		p_state->tokenizing_header = false;

	switch(token)
	{
		case TOKEN_EOF:
		case TOKEN_UNKNOWN:
			// this token was handled above
			assert(0);
			break;

		case TOKEN_PAYLOAD:
			std::printf("payload (length=%zu)\n", matchlen-1);
			break;

		case TOKEN_RECORD_SEPARATOR:
			p_state->tokenizing_header = true;
			break;

		case TOKEN_A_A:
			//Message(Topic::AUTHENTICATION, Action::AUTHENTICATE, true);
			break;

		case TOKEN_A_REQ:
			break;

		case TOKEN_E_A_S:
			//Message(Topic::EVENT, Action::SUBSCRIBE, true);
			break;

		case TOKEN_E_L:
			//Message(Topic::EVENT, Action::LISTEN);
			std::printf("Event Listen\n");
			break;
	};

	return token;
}
