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
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <sstream>

#include <buffer.hpp>
#include <client.hpp>
#include <error_handler.hpp>
#include <message.hpp>


namespace deepstream
{

void ErrorHandler::invalid_state_transition(client::State s, const Message& m)
{
	invalid_state_transition_impl(s, m);
}


void ErrorHandler::system_error()
{
	system_error_impl( errno );
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



void ErrorHandler::invalid_state_transition_impl(
	client::State s, const Message& msg)
{
	std::ostringstream ss;
	ss << "state=" << s << ", message header=" << msg.header();
	std::string string = ss.str();

	std::fprintf( stderr, "Invalid state transition [%s]\n", string.c_str() );
}


void ErrorHandler::system_error_impl(int error)
{
	char error_message[80];
	std::fill_n( error_message, sizeof(error_message), 0 );
	strerror_r( error, error_message, sizeof(error_message) );

	std::fprintf( stderr, "Error: %s\n", error_message );
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

}
