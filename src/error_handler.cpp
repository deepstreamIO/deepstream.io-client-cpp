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


void ErrorHandler::websocket_frame_too_big(const Buffer& buffer)
{
	websocket_frame_too_big_impl(buffer);
}


void ErrorHandler::invalid_websocket_frame_flags(int expected, int got)
{
	invalid_websocket_frame_flags_impl(expected, got);
}


void ErrorHandler::sudden_disconnect(const std::string& uri)
{
	sudden_disconnect_impl(uri);
}


void ErrorHandler::timeout(Topic topic, Action action)
{
	timeout_impl(topic, action);
}


void ErrorHandler::timeout(Topic topic, Action action, const std::string& name)
{
	timeout_impl(topic, action, name);
}



void ErrorHandler::invalid_state_transition_impl(
	client::State s, const Message& msg)
{
	std::ostringstream ss;
	ss << "state=" << s << ", message header=" << msg.header();
	std::string string = ss.str();

	std::fprintf( stderr, "Invalid state transition [%s]\n", string.c_str() );
}


void ErrorHandler::websocket_frame_too_big_impl(const Buffer& buffer)
{
	std::fprintf(
		stderr, "WebSocket frame too big [buffer size=%zu]\n", buffer.size()
	);
}


void ErrorHandler::invalid_websocket_frame_flags_impl(int expected, int got)
{
	std::fprintf(
		stderr,
		"Invalid WebSocket frame flags [expected=%d, got=%d]\n",
		expected, got
	);
}


void ErrorHandler::sudden_disconnect_impl(const std::string& uri)
{
	std::fprintf(
		stderr, "Sudden disconnect [uri=%s]\n", uri.c_str()
	);
}


void ErrorHandler::timeout_impl(Topic topic, Action action)
{
	std::stringstream ss;
	ss << "topic=" << topic << " action=" << action;
	std::string string = ss.str();

	std::fprintf( stderr, "Timeout [%s]\n", string.c_str() );
}


void ErrorHandler::timeout_impl(
	Topic topic, Action action, const std::string& name)
{
	std::stringstream ss;
	ss << "topic=" << topic << " action=" << action << " name='" << name << "'";
	std::string string = ss.str();

	std::fprintf( stderr, "Timeout [%s]\n", string.c_str() );
}

}
