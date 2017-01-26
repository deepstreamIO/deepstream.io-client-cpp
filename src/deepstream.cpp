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
#include <arpa/inet.h>
#include <cstdint>

#include <stdexcept>

#include <Poco/Exception.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>

#include <buffer.hpp>
#include <client.hpp>
#include <deepstream.hpp>
#include <error_handler.hpp>
#include <message.hpp>
#include <websockets.hpp>
#include <use.hpp>

#include <cassert>


namespace net = Poco::Net;


namespace deepstream
{

std::unique_ptr<Client> Client::make(
	const std::string& uri,
	std::unique_ptr<ErrorHandler> p_error_handler)
{
	assert( p_error_handler );

	std::unique_ptr<Client> p( new Client(uri, std::move(p_error_handler)) );

	p->session_.setTimeout( Poco::Timespan::SECONDS );

	return p;
}


Client::Client(
	const std::string& uri_string,
	std::unique_ptr<ErrorHandler> p_error_handler
) :
	state_( client::State::AWAIT_CONNECTION ),
	p_error_handler_( std::move(p_error_handler) ),
	uri_( uri_string ),
	session_( uri_.getHost(), uri_.getPort() ),
	request_(net::HTTPRequest::HTTP_GET, uri_string,net::HTTPRequest::HTTP_1_1),
	websocket_(session_, request_, response_)
{
	assert( p_error_handler_ );
}


Buffer Client::login(const std::string& auth)
{
	(void)auth;
	return Buffer();
}


void Client::close()
{
	websocket_.shutdown();
}



std::pair<Buffer, websockets::StatusCode> Client::receive_()
{
	const std::size_t MAX_PAYLOAD_SIZE = 4096;
	const std::size_t MAX_BUFFER_SIZE = 1000 * MAX_PAYLOAD_SIZE;

	const int frame_flags =
		net::WebSocket::FRAME_FLAG_FIN | net::WebSocket::FRAME_OP_TEXT;
	const int eof_flags =
		net::WebSocket::FRAME_FLAG_FIN | net::WebSocket::FRAME_OP_CLOSE;

	session_.setTimeout( 0 * Poco::Timespan::SECONDS );

	Buffer buffer;
	std::size_t num_bytes_read = 0;
	websockets::StatusCode status_code = websockets::StatusCode::NONE;


	while( num_bytes_read + MAX_PAYLOAD_SIZE <= MAX_BUFFER_SIZE )
	{
		assert( num_bytes_read <= buffer.size() );
		buffer.resize( num_bytes_read + MAX_PAYLOAD_SIZE );

		int flags = 0;
		int ret = 0;

		try
		{
			assert( buffer.size() - num_bytes_read >= MAX_PAYLOAD_SIZE );

			ret = websocket_.receiveFrame(
				&buffer[num_bytes_read], MAX_PAYLOAD_SIZE, flags
			);
		}
		catch(Poco::TimeoutException& e)
		{
			break;
		}
		catch(net::WebSocketException& e)
		{
			websocket_.shutdown();

			status_code = websockets::StatusCode::ABNORMAL_CLOSE;
			p_error_handler_->websocket_exception(e);

			break;
		}

		if( ret < 0 )
		{
			assert(0);
			throw std::logic_error("receiveFrame() returned a negative value");
		}

		if( ret == 0 )
		{
			websocket_.shutdown();

			if( status_code != websockets::StatusCode::NORMAL_CLOSE )
			{
				status_code = websockets::StatusCode::ABNORMAL_CLOSE;
				p_error_handler_->sudden_disconnect( uri_.toString() );
			}

			break;
		}

		if( !(flags == frame_flags) && !(flags == eof_flags && ret == 2) )
		{
			websocket_.shutdown();

			status_code = websockets::StatusCode::ABNORMAL_CLOSE;
			p_error_handler_->unexpected_websocket_frame_flags(flags);

			break;
		}

		if( flags == eof_flags && ret == 2 )
		{
			union {
				Buffer::value_type* p_data;
				std::uint16_t* p_uint16_t;
			} payload;

			payload.p_data = &buffer[num_bytes_read];
			std::uint16_t u16 = ntohs( payload.p_uint16_t[0] );

			status_code = (u16 == 1000)
				? websockets::StatusCode::NORMAL_CLOSE
				: websockets::StatusCode::UNKNOWN;

			continue;
		}

		assert( flags == frame_flags );

		num_bytes_read += ret;
	}

	buffer.resize(num_bytes_read);

	return std::make_pair(buffer, status_code);
}


void Client::send_(const Message& message)
{
	client::State new_state =
		client::transition(state_, message, Sender::CLIENT);
	use( new_state );
	assert( new_state != client::State::ERROR );

	Buffer buffer = message.to_binary();
	int ret = websocket_.sendFrame( buffer.data(), buffer.size() );

	if( ret == 0 )
		p_error_handler_->sudden_disconnect( uri_.toString() );

	if( ret < 0 )
		p_error_handler_->system_error();

	if( ret > 0 && std::size_t(ret) < buffer.size() )
	{
		assert(0);
		throw std::runtime_error("Incomplete message sent");
	}
}

}
