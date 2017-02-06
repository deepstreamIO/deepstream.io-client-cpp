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
#include <cstdint>

#include <algorithm>
#include <chrono>
#include <stdexcept>

#include <buffer.hpp>
#include <client.hpp>
#include <deepstream.hpp>
#include <error_handler.hpp>
#include <message.hpp>
#include <message_builder.hpp>
#include <scope_guard.hpp>
#include <websockets.hpp>
#include <websockets/poco.hpp>
#include <use.hpp>

#include <cassert>


namespace deepstream
{

Client Client::make(
	std::unique_ptr<websockets::Client> p_websocket,
	std::unique_ptr<ErrorHandler> p_error_handler)
{
	assert( p_websocket );
	assert( p_error_handler );

	const unsigned MAX_NUM_REDIRECTIONS = 3;

	for(unsigned num_redirections = 0;
		num_redirections < MAX_NUM_REDIRECTIONS;
		++num_redirections)
	{
		p_websocket->set_receive_timeout( std::chrono::seconds(1) );

		const std::string uri = p_websocket->uri();

		Client c(
			std::move(p_websocket),
			std::move(p_error_handler)
		);

		Buffer buffer;
		parser::MessageList messages;
		client::State& state = c.state_;

		if( c.receive_(&buffer, &messages) != websockets::State::OPEN )
			return c;

		if( messages.size() != 1 || state != client::State::CHALLENGING )
		{
			c.close();
			return c;
		}

		{
			MessageBuilder chr(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
			chr.add_argument(uri);

			if( c.send_(chr) != websockets::State::OPEN )
				return c;
		}

		if( c.receive_(&buffer, &messages) != websockets::State::OPEN )
			return c;

		if( messages.size() != 1 )
		{
			c.close();
			return c;
		}

		if( state == client::State::AWAIT_AUTHENTICATION )
			return c;

		if( state == client::State::AWAIT_CONNECTION )
		{
			const Message& redirect_msg = messages.front();
			assert( redirect_msg.num_arguments() == 1 );

			const Buffer& uri_buffer = redirect_msg[0];
			std::string new_uri( uri_buffer.cbegin(), uri_buffer.cend() );

			p_websocket = c.p_websocket_->construct(new_uri);
			p_error_handler = std::move(c.p_error_handler_);

			continue;
		}

		c.close();
		return c;
	}

	assert( p_error_handler );
	p_error_handler->too_many_redirections(MAX_NUM_REDIRECTIONS);

	return Client( nullptr, std::move(p_error_handler) );
}


Client Client::make(const std::string& uri)
{
	return Client::make(
		std::unique_ptr<websockets::Client>(new websockets::poco::Client(uri)),
		std::unique_ptr<ErrorHandler>(new ErrorHandler())
	);
}



Client::Client(
	std::unique_ptr<websockets::Client> p_websocket,
	std::unique_ptr<ErrorHandler> p_error_handler
) :
	event(
		[this] (const Message& message) -> bool {
			return send_(message) == websockets::State::OPEN;
		}
	),
	presence(
		[this] (const Message& message) -> bool {
			return send_(message) == websockets::State::OPEN;
		}
	),
	state_(
		p_websocket
		? client::State::AWAIT_CONNECTION
		: client::State::ERROR
	),
	p_websocket_( std::move(p_websocket) ),
	p_error_handler_( std::move(p_error_handler) )
{
	assert( p_error_handler_ );
}


client::State Client::login(
	const std::string& auth, Buffer* p_user_data)
{
	if( !p_websocket_ )
		return client::State::ERROR;

	if( state_ != client::State::AWAIT_AUTHENTICATION )
		throw std::logic_error("Cannot login() in current state");

	MessageBuilder areq(Topic::AUTH, Action::REQUEST);
	areq.add_argument(auth);

	if( send_(areq) != websockets::State::OPEN )
		return state_;

	Buffer buffer;
	parser::MessageList messages;

	if( receive_(&buffer, &messages) != websockets::State::OPEN )
		return state_;

	if( messages.size() != 1 )
	{
		close();
		return state_;
	}

	const Message& msg = messages.front();

	if( msg.topic() == Topic::AUTH &&
		msg.action() == Action::REQUEST &&
		msg.is_ack() )
	{
		assert( state_ == client::State::CONNECTED );

		if( msg.num_arguments() == 0 && p_user_data )
			p_user_data->clear();
		else if( msg.num_arguments() == 1 && p_user_data )
			*p_user_data = msg[0];

		return state_;
	}

	if( msg.topic() == Topic::AUTH &&
		msg.action() == Action::ERROR_INVALID_AUTH_DATA )
	{
		assert( state_ == client::State::AWAIT_AUTHENTICATION );

		p_error_handler_->authentication_error(msg);
		return state_;
	}

	if( msg.topic() == Topic::AUTH &&
		msg.action() == Action::ERROR_INVALID_AUTH_MSG )
	{
		p_error_handler_->authentication_error(msg);
		close();
		return state_;
	}

	if( msg.topic() == Topic::AUTH &&
		msg.action() == Action::ERROR_TOO_MANY_AUTH_ATTEMPTS )
	{
		p_error_handler_->authentication_error(msg);
		close();
		return state_;
	}


	close();
	return client::State::ERROR;
}


void Client::close()
{
	assert( p_websocket_ || state_ == client::State::ERROR );

	if( !p_websocket_ )
		return;

	state_ = client::State::DISCONNECTED;

		p_websocket_->close();
}



void Client::process_messages()
{
	if( !p_websocket_ )
		return;


	Buffer buffer;
	parser::MessageList messages;

	p_websocket_->set_receive_timeout( std::chrono::milliseconds(1) );

	while( receive_(&buffer, &messages) == websockets::State::OPEN )
	{
		if( messages.empty() )
			break;

		for(const Message& message : messages)
		{
			if( message.topic() == Topic::CONNECTION &&
				message.action() == Action::PING )
			{
				MessageBuilder pong( Topic::CONNECTION, Action::PONG );
				send_(pong);
				continue;
			}

			switch( message.topic() )
			{
				case Topic::EVENT:
					event.notify_(message);
					break;

				case Topic::PRESENCE:
					presence.notify_(message);
					break;

				default:
					assert(0);
					close();
					return;
			}
		}
	}
}



websockets::State Client::receive_(
	Buffer* p_buffer, parser::MessageList* p_messages)
{
	assert( p_buffer );
	assert( p_messages );

	if( !p_websocket_ )
		return websockets::State::ERROR;

	auto receive_ret = p_websocket_->receive_frame();
	websockets::State ws_state = receive_ret.first;
	std::unique_ptr<websockets::Frame> p_frame = std::move(receive_ret.second);

	if( ws_state == websockets::State::OPEN && !p_frame )
	{
		p_buffer->clear();
		p_messages->clear();

		return ws_state;
	}

	if( ws_state == websockets::State::CLOSED && p_frame )
	{
		p_buffer->clear();
		p_messages->clear();

		close();

		return ws_state;
	}

	if( ws_state == websockets::State::CLOSED && !p_frame )
	{
		p_buffer->clear();
		p_messages->clear();

		close();
		p_error_handler_->sudden_disconnect( p_websocket_->uri() );

		return ws_state;
	}

	if( ws_state == websockets::State::ERROR )
	{
		assert( p_frame );

		close();
		p_error_handler_->invalid_close_frame_size(*p_frame);

		return ws_state;
	}

	assert( ws_state == websockets::State::OPEN );
	assert( p_frame );

	if( p_frame->flags() !=
		(websockets::Frame::Bit::FIN | websockets::Frame::Opcode::TEXT_FRAME) )
	{
		p_error_handler_->unexpected_websocket_frame_flags( p_frame->flags() );
		close();

		return websockets::State::ERROR;
	}


	const Buffer& payload = p_frame->payload();
	p_buffer->assign( payload.cbegin(), payload.cend() );
	p_buffer->push_back(0);
	p_buffer->push_back(0);

	auto parser_ret = parser::execute( p_buffer->data(), p_buffer->size() );
	const parser::ErrorList& errors = parser_ret.second;

	std::for_each(
		errors.cbegin(), errors.cend(),
		[this] (const parser::Error& e) {
			this->p_error_handler_->parser_error(e);
		}
	);

	if( !errors.empty() )
	{
		p_buffer->clear();
		p_messages->clear();

		close();
		return websockets::State::ERROR;
	}


	*p_messages = std::move(parser_ret.first);

	for(auto it = p_messages->cbegin(); it != p_messages->cend(); ++it)
	{
		const Message& msg = *it;

		client::State old_state = state_;
		client::State new_state =
			client::transition(old_state, msg, Sender::SERVER);

		if( new_state == client::State::ERROR )
		{
			close();
			p_error_handler_->invalid_state_transition(old_state, msg);

			return websockets::State::ERROR;
		}

		if( new_state == client::State::DISCONNECTED )
			return websockets::State::OPEN;

		state_ = new_state;
	}

	return ws_state;
}


websockets::State Client::send_(const Message& message)
{
	if( !p_websocket_ )
		return websockets::State::ERROR;

	client::State new_state =
		client::transition(state_, message, Sender::CLIENT);
	assert( new_state != client::State::ERROR );

	if( new_state == client::State::ERROR )
		throw std::logic_error( "Invalid client state transition" );

	state_ = new_state;

	return send_frame_( message.to_binary() );
}



websockets::State Client::send_frame_(const Buffer& buffer)
{
	return send_frame_(
		buffer,
		websockets::Frame::Bit::FIN | websockets::Frame::Opcode::TEXT_FRAME
	);
}


websockets::State Client::send_frame_(
	const Buffer& buffer,
	websockets::Frame::Flags flags)
{
	if( !p_websocket_ )
		return websockets::State::ERROR;

	try
	{
		websockets::State state = p_websocket_->send_frame(buffer, flags);

		if( state != websockets::State::OPEN )
		{
			close();
			p_error_handler_->sudden_disconnect( p_websocket_->uri() );

			return websockets::State::CLOSED;
		}

		return state;
	}
	catch(std::system_error& e)
	{
		p_error_handler_->system_error(e);
	}
	catch(std::exception& e)
	{
		p_error_handler_->websocket_exception(e);
	}

	close();
	return websockets::State::ERROR;
}

}
