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
#include <cstdlib>
#include <cstring>

#include <message.hpp>
#include <parser.h>
#include <parser.hpp>
#include <scope_guard.hpp>
#include <use.hpp>

#include <cassert>


bool is_header_token(enum deepstream_token token)
{
	int min_event_num = TOKEN_A_A;

	return token >= min_event_num;
}



deepstream_parser_state::deepstream_parser_state(const char* p, std::size_t sz):
	buffer_(p),
	buffer_size_(sz),
	tokenizing_header_(true),
	offset_(0)
{
	assert(buffer_);
}




int deepstream_parser_handle(
	deepstream::parser::State* p_state, deepstream_token token,
	const char* text, std::size_t textlen)
{
	assert( p_state );
	assert( token != TOKEN_MAXVAL );

	return p_state->handle_token(token, text, textlen);
}



int deepstream_parser_state::handle_token(
	deepstream_token token, const char* text, std::size_t textlen)
{
	assert( token != TOKEN_MAXVAL );

	assert( text );
	assert( textlen > 0 );

	assert( token != TOKEN_EOF || *text == '\0' );
	assert( token != TOKEN_EOF || textlen == 1 );
	assert( token != TOKEN_EOF || offset_ + textlen == buffer_size_ + 1 );
	assert( token == TOKEN_EOF || offset_ + textlen <= buffer_size_ );
	assert( token == TOKEN_EOF || !std::memcmp(buffer_+offset_, text, textlen));

	assert( messages_.size() <= offset_ );
	assert( errors_.size() <= offset_ );


	DEEPSTREAM_ON_EXIT([this, textlen] () {
		this->offset_ += textlen;
	} );


	if(token == TOKEN_UNKNOWN)
		handle_error(token, text, textlen);
	else if(token == TOKEN_EOF)
	{
		assert(offset_ == buffer_size_);

		if(!tokenizing_header_)
			handle_error(token, text, textlen);
	}
	else if(token == TOKEN_PAYLOAD)
		handle_payload(token, text, textlen);
	else if(token == TOKEN_MESSAGE_SEPARATOR)
		handle_message_separator(token, text, textlen);
	else if( is_header_token(token) )
		handle_header(token, text, textlen);
	else
	{
		assert(0);
	}

	return token;
}


void deepstream_parser_state::handle_error(
	deepstream_token token, const char*, std::size_t textlen)
{
	using deepstream::parser::Error;

	assert( token == TOKEN_EOF || token == TOKEN_UNKNOWN );
	assert( textlen > 0 || (textlen == 0 && token == EOF) );


	DEEPSTREAM_ON_EXIT( [this] () {
		this->tokenizing_header_ = true; // reset parser status on exit
	} );


	if( token == TOKEN_EOF )
	{
		assert( !tokenizing_header_ );
		assert( !messages_.empty() );

		messages_.pop_back();
		errors_.emplace_back(offset_, textlen, Error::UNEXPECTED_EOF);
	}


	if( token == TOKEN_UNKNOWN && tokenizing_header_ )
	{
		errors_.emplace_back(offset_, textlen, Error::UNEXPECTED_TOKEN);
	}

	if( token == TOKEN_UNKNOWN && !tokenizing_header_ )
	{
		assert( !messages_.empty() );

		std::size_t msg_start = messages_.back().offset();
		std::size_t msg_size = messages_.back().size();
		messages_.pop_back();

		assert( msg_start + msg_size == offset_ );

		errors_.emplace_back(
			msg_start, msg_size+textlen, Error::CORRUPT_MESSAGE);
	}
}


#define DS_ADD_MSG(...) \
		do { \
			messages_.emplace_back(buffer_, offset_, __VA_ARGS__); \
		} while(false)


void deepstream_parser_state::handle_header(
	deepstream_token token, const char* text, std::size_t textlen)
{
	using deepstream::Topic;
	using deepstream::Action;

	assert( is_header_token(token) );
	assert( tokenizing_header_ );

	DEEPSTREAM_ON_EXIT( [this] () {
		this->tokenizing_header_ = false;
	} );

	switch(token)
	{
		// avoid compiler warnings [-Wswitch]
		case TOKEN_EOF:
		case TOKEN_UNKNOWN:
		case TOKEN_PAYLOAD:
		case TOKEN_MESSAGE_SEPARATOR:
		case TOKEN_MAXVAL:
			assert(0);
			break;

		case TOKEN_A_A:
			DS_ADD_MSG(Topic::AUTH, Action::REQUEST, true);
			break;

		case TOKEN_A_E_IAD:
			DS_ADD_MSG(Topic::AUTH, Action::ERROR_INVALID_AUTH_DATA, true);
			break;

		case TOKEN_A_E_TMAA:
			DS_ADD_MSG(Topic::AUTH, Action::ERROR_TOO_MANY_AUTH_ATTEMPTS);
			break;

		case TOKEN_A_REQ:
			DS_ADD_MSG(Topic::AUTH, Action::REQUEST);
			break;

		case TOKEN_C_A:
			DS_ADD_MSG(Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true);
			break;

		case TOKEN_C_CH:
			DS_ADD_MSG(Topic::CONNECTION, Action::CHALLENGE);
			break;

		case TOKEN_C_CHR:
			DS_ADD_MSG(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
			break;

		case TOKEN_C_RED:
			DS_ADD_MSG(Topic::CONNECTION, Action::REDIRECT);
			break;

		case TOKEN_C_REJ:
			DS_ADD_MSG(Topic::CONNECTION, Action::REJECT);

		case TOKEN_E_A_L:
			DS_ADD_MSG(Topic::EVENT, Action::LISTEN, true);
			break;

		case TOKEN_E_A_S:
			DS_ADD_MSG(Topic::EVENT, Action::SUBSCRIBE, true);
			break;

		case TOKEN_E_L:
			DS_ADD_MSG(Topic::EVENT, Action::LISTEN);
			break;

		case TOKEN_E_S:
			DS_ADD_MSG(Topic::EVENT, Action::SUBSCRIBE);
			break;

		case TOKEN_E_US:
			DS_ADD_MSG(Topic::EVENT, Action::UNSUBSCRIBE);
			break;
	}

#ifndef NDEBUG
	const auto& msg = messages_.back();
	const char* p = msg.header().to_string();
	std::vector<char> bin = deepstream::Message::from_human_readable(p);

	assert( textlen == msg.header().size() );
	assert( textlen == bin.size() );
	assert( std::equal(bin.cbegin(), bin.cend(), text) );
#endif
}


void deepstream_parser_state::handle_payload(
	deepstream_token token, const char* text, std::size_t textlen)
{
	using namespace deepstream::parser;

	assert( token == TOKEN_PAYLOAD );
	assert( text );
	assert( textlen > 0 );
	assert( text[0] == ASCII_UNIT_SEPARATOR );
	assert( !messages_.empty() );

	deepstream::use(token);
	deepstream::use(text);

	auto& msg = messages_.back();
	msg.arguments_.emplace_back(offset_+1, textlen-1);
	msg.size_ += textlen;
}


void deepstream_parser_state::handle_message_separator(
	deepstream_token token, const char* text, std::size_t textlen)
{
	using namespace deepstream::parser;

	assert( token == TOKEN_MESSAGE_SEPARATOR );
	assert( text );
	assert( textlen == 1 );
	assert( text[0] == ASCII_RECORD_SEPARATOR );
	assert( !messages_.empty() );

	deepstream::use(token);
	deepstream::use(text);

	DEEPSTREAM_ON_EXIT( [this] () {
		this->tokenizing_header_ = true;
	} );


	auto& msg = messages_.back();
	msg.size_ += textlen;

	std::size_t msg_offset = msg.offset();
	std::size_t msg_size = msg.size();
	std::size_t num_args = msg.arguments_.size();
	const auto& expected_num_args = msg.num_arguments();
	std::size_t min_num_args = expected_num_args.first;
	std::size_t max_num_args = expected_num_args.second;

	if( num_args >= min_num_args && num_args <= max_num_args )
		return;

	messages_.pop_back();
	errors_.emplace_back(
		msg_offset, msg_size, Error::INVALID_NUMBER_OF_ARGUMENTS);
}
