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
#include <error_handler.hpp>
#include <message.hpp>
#include <deepstream.hpp>
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



Buffer Client::receive_()
{
	Buffer buffer(1024, 0);
	int flags = 0;
	int ret = 0;

	session_.setTimeout( 0 * Poco::Timespan::SECONDS );

	try
	{
		ret = websocket_.receiveFrame( buffer.data(), buffer.size()-2, flags );
	}
	catch(Poco::TimeoutException& e)
	{
		// do nothing
	}
	catch(net::WebSocketException& e)
	{
		p_error_handler_->websocket_exception(e);
		buffer.resize(0);
		return buffer;
	}

	if( ret == 0 )
	{
		p_error_handler_->sudden_disconnect( uri_.toString() );
		buffer.resize(0);
		return buffer;
	}

	if( !(flags & net::WebSocket::FRAME_FLAG_FIN) ||
		 (flags & net::WebSocket::FRAME_OP_CONT) ||
		!(flags & net::WebSocket::FRAME_OP_TEXT) )
	{
		int expected =
			net::WebSocket::FRAME_FLAG_FIN | net::WebSocket::FRAME_OP_TEXT;

		p_error_handler_->invalid_websocket_frame_flags(expected, flags);

		buffer.resize(0);
		return buffer;
	}

	return buffer;
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
