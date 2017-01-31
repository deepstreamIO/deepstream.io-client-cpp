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
#include <exception.hpp>
#include <message.hpp>
#include <message_builder.hpp>
#include <scope_guard.hpp>
#include <websockets.hpp>
#include <websockets/poco.hpp>
#include <use.hpp>

#include <cassert>


namespace deepstream
{

std::unique_ptr<Client> Client::make(
	const std::string& uri,
	std::unique_ptr<ErrorHandler> p_error_handler)
{
	assert( p_error_handler );

	std::unique_ptr<websockets::Client> p_websocket(
		new websockets::poco::Client(uri)
	);

	p_websocket->set_receive_timeout( std::chrono::seconds(1) );

	std::unique_ptr<Client> p(
		new Client(
			std::move(p_websocket),
			std::move(p_error_handler)
		)
	);

	Buffer buffer;
	parser::MessageList messages;
	client::State& state = p->state_;
	if( p->receive_(&buffer, &messages) != websockets::State::OPEN )
		return p;

	assert( messages.size() == 1 );
	assert( state == client::State::CHALLENGING );

	{
		MessageBuilder chr(Topic::CONNECTION, Action::CHALLENGE_RESPONSE);
		chr.add_argument( Buffer(uri.cbegin(), uri.cend()) );

		if( p->send_(chr) != websockets::State::OPEN )
			return p;
	}

	if( p->receive_(&buffer, &messages) != websockets::State::OPEN )
		return p;

	assert( messages.size() == 1 );
	assert( state == client::State::AWAIT_AUTHENTICATION );

	return p;
}


Client::Client(
	std::unique_ptr<websockets::Client> p_websocket,
	std::unique_ptr<ErrorHandler> p_error_handler
) :
	state_(client::State::AWAIT_CONNECTION),
	p_websocket_( std::move(p_websocket) ),
	p_error_handler_( std::move(p_error_handler) )
{
	assert( p_websocket_ );
	assert( p_error_handler_ );
}


client::State Client::login(
	const std::string& auth, Buffer* p_user_data)
{
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

	assert( messages.size() == 1 );
	assert( state_ == client::State::CONNECTED );

	const Message& aa_msg = messages.front();
	assert( aa_msg.topic() == Topic::AUTH );
	assert( aa_msg.action() == Action::REQUEST );
	assert( aa_msg.is_ack() );
	assert( aa_msg.num_arguments() >= 0 );
	assert( aa_msg.num_arguments() <= 1 );

	if( aa_msg.num_arguments() == 0 && p_user_data )
		p_user_data->clear();
	else if( aa_msg.num_arguments() == 1 && p_user_data )
		*p_user_data = aa_msg[0];

	return state_;
}


void Client::close()
{
	state_ = client::State::DISCONNECTED;
	p_websocket_->close();
}


websockets::State Client::receive_(
	Buffer* p_buffer, parser::MessageList* p_messages)
{
	assert( p_buffer );
	assert( p_messages );

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
		assert( new_state != client::State::DISCONNECTED );

		if( new_state == client::State::ERROR )
		{
			close();
			p_error_handler_->invalid_state_transition(old_state, msg);

			return websockets::State::ERROR;
		}

		state_ = new_state;
	}

	return ws_state;
}


websockets::State Client::send_(const Message& message)
{
	client::State new_state =
		client::transition(state_, message, Sender::CLIENT);
	assert( new_state != client::State::ERROR );

	if( new_state == client::State::ERROR )
		throw std::logic_error( "Invalid client state transition" );

	state_ = new_state;

	try
	{
		websockets::State state = p_websocket_->send_frame(message.to_binary());

		if( state != websockets::State::OPEN )
		{
			close();
			p_error_handler_->sudden_disconnect( p_websocket_->uri() );

			return websockets::State::CLOSED;
		}

		return state;
	}
	catch(SystemError& e)
	{
		p_error_handler_->system_error( e.error() );
	}
	catch(std::exception& e)
	{
		p_error_handler_->websocket_exception(e);
	}

	close();
	return websockets::State::ERROR;
}

}
