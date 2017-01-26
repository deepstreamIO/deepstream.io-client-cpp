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
#include <cstddef>
#include <cstdio>

#include <algorithm>
#include <sstream>
#include <vector>

#include <arpa/inet.h>

#include <Poco/Exception.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/NetException.h>

#include <buffer.hpp>
#include <client.hpp>
#include <message.hpp>
#include <message_builder.hpp>
#include <parser.hpp>
#include <scope_guard.hpp>

#include <cassert>


namespace net = Poco::Net;
namespace ds = deepstream;



int main()
try
{
	net::HTTPClientSession session("localhost", 6020);
	session.setTimeout( Poco::Timespan::SECONDS );

	net::HTTPResponse response;
	net::HTTPRequest request(
		net::HTTPRequest::HTTP_GET, "/deepstream", net::HTTPRequest::HTTP_1_1
	);

	net::WebSocket ws(session, request, response);

	std::printf( "receive time-out: %ds\n", ws.getReceiveTimeout().seconds() );
	std::printf( "send time-out: %ds\n", ws.getSendTimeout().seconds() );


	ds::client::State state = ds::client::State::AWAIT_CONNECTION;

	std::vector<char> buffer(160, 0);
	int flags = 0;
	int ret = 0;

	// receive C|CH+
	{
		try
		{
			ret = ws.receiveFrame( buffer.data(), buffer.size()-2, flags );
		}
		catch(net::WebSocketException& e)
		{
			ws.shutdown();
			throw;
		}

		if( ret == 0 )
		{
			std::fprintf( stderr, "Sudden disconnect\n" );
			return 0;
		}

		if( !(flags & net::WebSocket::FRAME_FLAG_FIN) )
		{
			std::fprintf( stderr, "Error: received multi-frame message\n" );
			return 1;
		}
		if( flags & net::WebSocket::FRAME_OP_CONT )
		{
			std::fprintf( stderr, "Error: received continuation frame\n" );
			return 1;
		}
		if( !(flags & net::WebSocket::FRAME_OP_TEXT) )
		{
			std::fprintf( stderr, "Error: received non-text frame\n" );
			return 1;
		}

		auto contents = deepstream::parser::execute( buffer.data(), ret + 2 );
		const auto& messages = contents.first;
		const auto& errors = contents.second;

		for(auto it = errors.cbegin(); it != errors.cend(); ++it)
		{
			std::stringstream ss;
			ss << *it;
			const auto error_string = ss.str();

			std::fprintf( stderr, "Parser error: %s\n", error_string.c_str() );
		}

		for(auto it = messages.cbegin(); it != messages.cend(); ++it)
		{
			ds::client::State new_state =
				ds::client::transition(state, *it, ds::Sender::SERVER);

			if( new_state == ds::client::State::ERROR )
			{
				std::fprintf( stderr, "State error\n" );
				return 2;
			}

			state = new_state;
		}

		printf( "state: %d\n", static_cast<int>(state) );
	}


	// send C|CHR|url+
	{
		ds::MessageBuilder chr_builder(
			ds::Topic::CONNECTION, ds::Action::CHALLENGE_RESPONSE
		);
		chr_builder.add_argument( ds::Buffer("ws://localhost:6020/deepstream") );
		ds::Buffer chr = chr_builder.to_binary();

		ret = ws.sendFrame( &chr[0], chr.size() );

		state = ds::client::transition(state, chr_builder, ds::Sender::CLIENT);
		printf( "state: %d\n", static_cast<int>(state) );
	}


	// receive C|A+
	{
		std::fill( buffer.begin(), buffer.end(), 0 );

		try
		{
			ret = ws.receiveFrame( buffer.data(), buffer.size()-2, flags );
		}
		catch(net::WebSocketException& e)
		{
			ws.shutdown();
			throw;
		}

		if( ret == 0 )
		{
			std::fprintf( stderr, "Sudden disconnect\n" );
			return 0;
		}

		if( !(flags & net::WebSocket::FRAME_FLAG_FIN) )
		{
			std::fprintf( stderr, "Error: received multi-frame message\n" );
			return 1;
		}
		if( flags & net::WebSocket::FRAME_OP_CONT )
		{
			std::fprintf( stderr, "Error: received continuation frame\n" );
			return 1;
		}
		if( !(flags & net::WebSocket::FRAME_OP_TEXT) )
		{
			std::fprintf( stderr, "Error: received non-text frame\n" );
			return 1;
		}

		auto contents = deepstream::parser::execute( buffer.data(), ret + 2 );
		const auto& messages = contents.first;
		const auto& errors = contents.second;

		for(auto it = errors.cbegin(); it != errors.cend(); ++it)
		{
			std::stringstream ss;
			ss << *it;
			const auto error_string = ss.str();

			std::fprintf( stderr, "Parser error: %s\n", error_string.c_str() );
		}

		for(auto it = messages.cbegin(); it != messages.cend(); ++it)
		{
			ds::client::State new_state =
				ds::client::transition(state, *it, ds::Sender::SERVER);

			if( new_state == ds::client::State::ERROR )
			{
				std::fprintf( stderr, "State error\n" );
				return 2;
			}

			state = new_state;
		}

		printf( "state: %d\n", static_cast<int>(state) );
	}


	// send A|REQ|auth+
	{
		ds::MessageBuilder auth_builder( ds::Topic::AUTH, ds::Action::REQUEST );
		auth_builder.add_argument(
			ds::Buffer("{ \"username\": \"foo\", \"password\": \"bar\" }")
		);
		ds::Buffer auth = auth_builder.to_binary();

		ret = ws.sendFrame( &auth[0], auth.size() );

		state = ds::client::transition(state, auth_builder, ds::Sender::CLIENT);
		printf( "state: %d\n", static_cast<int>(state) );
	}


	// receive A|A+
	{
		std::fill( buffer.begin(), buffer.end(), 0 );

		try
		{
			ret = ws.receiveFrame( buffer.data(), buffer.size()-2, flags );
		}
		catch(net::WebSocketException& e)
		{
			ws.shutdown();
			throw;
		}

		if( ret == 0 )
		{
			std::fprintf( stderr, "Sudden disconnect\n" );
			return 0;
		}

		if( !(flags & net::WebSocket::FRAME_FLAG_FIN) )
		{
			std::fprintf( stderr, "Error: received multi-frame message\n" );
			return 1;
		}
		if( flags & net::WebSocket::FRAME_OP_CONT )
		{
			std::fprintf( stderr, "Error: received continuation frame\n" );
			return 1;
		}
		if( !(flags & net::WebSocket::FRAME_OP_TEXT) )
		{
			std::fprintf( stderr, "Error: received non-text frame\n" );
			return 1;
		}

		auto contents = deepstream::parser::execute( buffer.data(), ret + 2 );
		const auto& messages = contents.first;
		const auto& errors = contents.second;

		for(auto it = errors.cbegin(); it != errors.cend(); ++it)
		{
			std::stringstream ss;
			ss << *it;
			const auto error_string = ss.str();

			std::fprintf( stderr, "Parser error: %s\n", error_string.c_str() );
		}

		for(auto it = messages.cbegin(); it != messages.cend(); ++it)
		{
			ds::client::State new_state =
				ds::client::transition(state, *it, ds::Sender::SERVER);

			if( new_state == ds::client::State::ERROR )
			{
				std::fprintf( stderr, "State error\n" );
				return 2;
			}

			state = new_state;
		}

		printf( "state: %d\n", static_cast<int>(state) );
	}


	// disconnect
	ws.shutdown();


	// receive socket EOF
	{
		std::fill( buffer.begin(), buffer.end(), 0 );

		try
		{
			ret = ws.receiveFrame( buffer.data(), buffer.size()-2, flags );
		}
		catch(net::WebSocketException& e)
		{
			ws.shutdown();
			throw;
		}

		if( ret == 0 )
		{
			std::fprintf( stderr, "Sudden disconnect\n" );
			return 0;
		}

		if( !(flags & net::WebSocket::FRAME_FLAG_FIN) )
		{
			std::fprintf( stderr, "Error: received multi-frame message\n" );
			return 1;
		}
		if( flags & net::WebSocket::FRAME_OP_CONT )
		{
			std::fprintf( stderr, "Error: received continuation frame\n" );
			return 1;
		}
		if( !(flags & net::WebSocket::FRAME_OP_CLOSE) )
		{
			std::fprintf( stderr, "Error: received non-close frame\n" );
			return 1;
		}

		if( ret != 2 )
		{
			std::fprintf( stderr, "Error: EOF payload size is %d\n", ret );
			return 1;
		}

		union {
			char* p_c;
			unsigned short* p_us;
		} mine;

		mine.p_c = &buffer[0];
		unsigned short code = ntohs(*mine.p_us);

		if( code != net::WebSocket::WS_NORMAL_CLOSE )
		{
			std::fprintf( stderr, "Error: close code %d\n", code );
			return 1;
		}

		state = ds::client::State::DISCONNECTED;
	}
}
catch(net::ConnectionRefusedException& e)
{
	std::fprintf( stderr, "%s\n", e.what() );
}
catch(Poco::TimeoutException& e)
{
	std::fprintf( stderr, "Error: timeout\n" );
}
