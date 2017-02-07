/*
 * Copyright 2017 deepstreamHub GmbH
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
#include <cstring>

#include <algorithm>
#include <sstream>

#include <deepstream/buffer.hpp>
#include <deepstream/client.hpp>
#include <deepstream/error_handler.hpp>
#include <deepstream/message.hpp>
#include <deepstream/parser.hpp>
#include <deepstream/websockets.hpp>


namespace deepstream
{

void ErrorHandler::parser_error(const parser::Error& e)
{
	parser_error_impl(e);
}


void ErrorHandler::invalid_state_transition(client::State s, const Message& m)
{
	invalid_state_transition_impl(s, m);
}


void ErrorHandler::too_many_redirections(unsigned max_num_redirections)
{
	assert( max_num_redirections > 0 );

	too_many_redirections_impl(max_num_redirections);
}


void ErrorHandler::system_error(const std::system_error& e)
{
	system_error_impl(e);
}


void ErrorHandler::invalid_close_frame_size(const websockets::Frame& frame)
{
	assert( frame.payload().size() != 2 );

	invalid_close_frame_size_impl(frame);
}


void ErrorHandler::websocket_exception(const std::exception& e)
{
	websocket_exception_impl(e);
}


void ErrorHandler::unexpected_websocket_frame_flags(int flags)
{
	unexpected_websocket_frame_flags_impl(flags);
}


void ErrorHandler::sudden_disconnect(const std::string& uri)
{
	sudden_disconnect_impl(uri);
}


void ErrorHandler::authentication_error(const Message& message)
{
	assert( message.topic() == Topic::AUTH );
	assert(
		message.action() == Action::ERROR_INVALID_AUTH_DATA ||
		message.action() == Action::ERROR_INVALID_AUTH_MSG ||
		message.action() == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS
	);
	assert( message.num_arguments() == 1 );

	authentication_error_impl(message);
}



void ErrorHandler::parser_error_impl(const parser::Error& e)
{
	std::ostringstream os;
	os << e.tag();
	std::string string = os.str();

	std::fprintf( stderr, "Parser error [error=%s]\n", string.c_str() );
}


void ErrorHandler::invalid_state_transition_impl(
	client::State s, const Message& msg)
{
	std::ostringstream ss;
	ss << "state=" << s << ", message header=" << msg.header();
	std::string string = ss.str();

	std::fprintf( stderr, "Invalid state transition [%s]\n", string.c_str() );
}


void ErrorHandler::too_many_redirections_impl(unsigned max_num_redirections)
{
	std::fprintf(
		stderr,
		"Too many redirections (%u redirections)\n",
		max_num_redirections
	);
}


void ErrorHandler::system_error_impl(const std::system_error& e)
{
	std::fprintf(
		stderr,
		"System error: %s [code=%d]\n",
		e.what(), e.code().value()
	);
}


void ErrorHandler::invalid_close_frame_size_impl(const websockets::Frame& frame)
{
	std::fprintf(
		stderr,
		"Invalid payload size in close frame [expected=2, got=%zu]\n",
		frame.payload().size()
	);
}


void ErrorHandler::websocket_exception_impl(const std::exception& e)
{
	std::fprintf(
		stderr, "WebSocket exception [msg=%s]\n", e.what()
	);
}


void ErrorHandler::unexpected_websocket_frame_flags_impl(int flags)
{
	std::fprintf(
		stderr, "Unexpected WebSocket frame flags [flags=%d]\n", flags
	);
}


void ErrorHandler::sudden_disconnect_impl(const std::string& uri)
{
	std::fprintf(
		stderr, "Sudden disconnect [uri=%s]\n", uri.c_str()
	);
}


void ErrorHandler::authentication_error_impl(const Message& msg)
{
	const Buffer& reason = msg[0];
	std::string str( reason.cbegin(), reason.cend() );

	std::fprintf(
		stderr, "Authentication error: '%s'\n", str.c_str()
	);
}

}
