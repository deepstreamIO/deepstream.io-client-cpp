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
#ifndef DEEPSTREAM_ERROR_HANDLER_HPP
#define DEEPSTREAM_ERROR_HANDLER_HPP

#include <stdexcept>
#include <string>


namespace deepstream
{
	enum class Topic;
	enum class Action;

	struct Buffer;
	struct Message;

	namespace client
	{
		enum class State;
	}

	namespace parser
	{
		struct Error;
	}

	namespace websockets
	{
		struct Frame;
	}


	struct ErrorHandler
	{
		virtual ~ErrorHandler() {};

		void parser_error(const parser::Error&);
		void invalid_state_transition(client::State, const Message&);

		void system_error(int error);

		void invalid_close_frame_size(const websockets::Frame&);
		void websocket_exception(const std::exception&);
		void unexpected_websocket_frame_flags(int got);
		void sudden_disconnect(const std::string& uri);

	protected:
		virtual void parser_error_impl(const parser::Error&);
		virtual void invalid_state_transition_impl(
			client::State, const Message&);

		virtual void system_error_impl(int error);

		virtual void invalid_close_frame_size_impl(const websockets::Frame&);
		virtual void websocket_exception_impl(const std::exception&);
		virtual void unexpected_websocket_frame_flags_impl(int);
		virtual void sudden_disconnect_impl(const std::string& uri);
	};
}

#endif
