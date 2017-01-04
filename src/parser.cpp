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
#include <cstdio>
#include <cstdlib>

#include <parser.h>
#include <parser.hpp>

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
	using deepstream::Topic;
	using deepstream::Action;
	using deepstream::parser::Error;

	assert( p_state );
	assert( match );
	assert( matchlen > 0 );

	const char* buffer = p_state->buffer;
	std::size_t offset = p_state->offset;
	bool tokenizing_header = p_state->tokenizing_header;
	auto& messages = p_state->messages;
	auto& errors = p_state->errors;

	p_state->offset = offset + matchlen;

	if( (tokenizing_header && !is_header_token(token)) ||
		(!tokenizing_header && is_header_token(token)) )
	{
		errors.emplace_back(offset, matchlen, Error::UNEXPECTED_TOKEN);
		return token;
	}


	// handle legal tokens
	if( tokenizing_header && is_header_token(token) )
		p_state->tokenizing_header = false;

	switch(token)
	{
		case TOKEN_EOF:
			if(!tokenizing_header)
				errors.emplace_back(
					offset, matchlen, Error::UNEXPECTED_EOF);
			break;

		case TOKEN_UNKNOWN:
			errors.emplace_back(
				offset, matchlen, Error::UNEXPECTED_TOKEN);
			break;


		case TOKEN_PAYLOAD:
			// TODO
			break;

		case TOKEN_RECORD_SEPARATOR:
			p_state->tokenizing_header = true;
			break;


		case TOKEN_A_A:
			messages.emplace_back(
				buffer, Topic::AUTH, Action::REQUEST, true);
			break;

		case TOKEN_A_E_IAD:
			messages.emplace_back(
				buffer, Topic::AUTH, Action::ERROR_INVALID_AUTH_DATA, true);
			break;

		case TOKEN_A_E_TMAA:
			messages.emplace_back(
				buffer, Topic::AUTH, Action::ERROR_TOO_MANY_AUTH_ATTEMPTS,true);
			break;

		case TOKEN_A_REQ:
			messages.emplace_back(
				buffer, Topic::AUTH, Action::REQUEST);
			break;

		case TOKEN_E_A_L:
			messages.emplace_back(
				buffer, Topic::EVENT, Action::LISTEN, true);
			break;

		case TOKEN_E_A_S:
			messages.emplace_back(
				buffer, Topic::EVENT, Action::SUBSCRIBE, true);
			break;

		case TOKEN_E_L:
			messages.emplace_back(
				buffer, Topic::EVENT, Action::LISTEN);
			break;

		case TOKEN_E_S:
			messages.emplace_back(
				buffer, Topic::EVENT, Action::SUBSCRIBE);
			break;

		case TOKEN_E_US:
			messages.emplace_back(
				buffer, Topic::EVENT, Action::UNSUBSCRIBE);
			break;
	};

	return token;
}
