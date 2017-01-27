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

#include <algorithm>
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
#include <message_builder.hpp>
#include <scope_guard.hpp>
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
	using StatusCode = websockets::StatusCode;

	assert( p_error_handler );

	std::unique_ptr<Client> p( new Client(uri, std::move(p_error_handler)) );

	p->session_.setTimeout( Poco::Timespan::SECONDS );

	Buffer buffer;
	parser::MessageList messages;
	client::State& state = p->state_;
	StatusCode sc = p->receive_messages_(&buffer, &messages);

	if( sc != StatusCode::NONE )
		return nullptr;

	for(const Message& msg : messages)
	{
		state = client::transition(state, msg, Sender::SERVER);

		if( state == client::State::ERROR )
		{
			p->close();
			p->p_error_handler_->invalid_state_transition(state, msg);
			return nullptr;
		}
	}

	assert( messages.size() == 1 );
	assert( state == client::State::CHALLENGING );

	{
		MessageBuilder chr(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
		chr.add_argument( Buffer(uri.cbegin(), uri.cend()) );

		state = client::transition(state, chr, Sender::CLIENT);
		assert( state == client::State::CHALLENGING_WAIT );

		p->send_(chr);
	}

	sc = p->receive_messages_(&buffer, &messages);

	if( sc != StatusCode::NONE )
		return nullptr;

	for(const Message& msg : messages)
	{
		state = client::transition(state, msg, Sender::SERVER);

		if( state == client::State::ERROR )
		{
			p->close();
			p->p_error_handler_->invalid_state_transition(state, msg);
			return nullptr;
		}
	}

	if( state != client::State::AWAIT_AUTHENTICATION )
		return nullptr;

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
	state_ = client::State::DISCONNECTED;
	websocket_.shutdown();
}



websockets::StatusCode Client::receive_messages_(
	Buffer* p_buffer, parser::MessageList* p_messages)
{
	assert( p_buffer );
	assert( p_messages );

	using StatusCode = websockets::StatusCode;

	StatusCode status_code = receive_(p_buffer);
	Buffer& buffer = *p_buffer;

	if( status_code == StatusCode::ABNORMAL_CLOSE )
		return status_code;

	if( buffer.empty() )
		return status_code;

	buffer.push_back(0);
	buffer.push_back(0);
	auto parser_ret = parser::execute( buffer.data(), buffer.size() );
	const parser::ErrorList& errors = parser_ret.second;

	std::for_each(
		errors.cbegin(), errors.cend(),
		[this] (const parser::Error& e) {
			this->p_error_handler_->parser_error(e);
		}
	);

	*p_messages = std::move(parser_ret.first);

	return status_code;
}


websockets::StatusCode Client::receive_(Buffer* p_buffer)
{
	assert( p_buffer );

	using StatusCode = websockets::StatusCode;

	const int frame_flags =
		net::WebSocket::FRAME_FLAG_FIN | net::WebSocket::FRAME_OP_TEXT;
	const int eof_flags =
		net::WebSocket::FRAME_FLAG_FIN | net::WebSocket::FRAME_OP_CLOSE;

	const std::size_t MAX_BUFFER_SIZE = 1u << 20;
	std::size_t num_bytes_available = websocket_.available();
	std::size_t buffer_size = std::min( MAX_BUFFER_SIZE, num_bytes_available );

	Buffer& buffer = *p_buffer;
	buffer.resize(buffer_size);
	std::size_t num_bytes_read = 0;

	auto do_return( [&num_bytes_read, &buffer] (StatusCode sc) {
		buffer.resize(num_bytes_read);
		return sc;
	} );


	session_.setTimeout( 0 * Poco::Timespan::SECONDS );

	while( num_bytes_read < buffer.size() )
	{
		int flags = 0;
		int ret = 0;

		try
		{
			ret = websocket_.receiveFrame(
				&buffer[num_bytes_read], buffer.size()-num_bytes_read, flags
			);
		}
		catch(Poco::TimeoutException& e)
		{
			return do_return(StatusCode::NORMAL_CLOSE);
		}
		catch(net::WebSocketException& e)
		{
			close();
			p_error_handler_->websocket_exception(e);

			return do_return(StatusCode::ABNORMAL_CLOSE);
		}

		if( ret < 0 )
		{
			assert(0);
			throw std::logic_error("receiveFrame() returned a negative value");
		}

		if( ret == 0 )
		{
			close();
			p_error_handler_->sudden_disconnect( uri_.toString() );

			return do_return(StatusCode::ABNORMAL_CLOSE);
		}

		if( !(flags == frame_flags) && !(flags == eof_flags && ret == 2) )
		{
			close();
			p_error_handler_->unexpected_websocket_frame_flags(flags);

			return do_return(StatusCode::ABNORMAL_CLOSE);
		}

		if( flags == eof_flags && ret == 2 )
		{
			close();

			union {
				Buffer::value_type* p_data;
				std::uint16_t* p_uint16_t;
			} payload;

			payload.p_data = &buffer[num_bytes_read];
			std::uint16_t u16 = ntohs( payload.p_uint16_t[0] );

			StatusCode status_code = (u16 == 1000)
				? StatusCode::NORMAL_CLOSE
				: StatusCode::UNKNOWN;

			return do_return(status_code);
		}

		assert( flags == frame_flags );

		num_bytes_read += ret;
	}

	return do_return(StatusCode::NONE);
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
