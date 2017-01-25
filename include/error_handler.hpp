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

	namespace websockets
	{
		struct Frame;
	}


	struct ErrorHandler
	{
		virtual ~ErrorHandler() {};

		void invalid_state_transition(client::State, const Message&);
		void websocket_frame_too_big(const Buffer& buffer);
		void invalid_websocket_frame_flags(int expected, int got);
		void sudden_disconnect(const std::string& uri);
		void timeout(Topic, Action);
		void timeout(Topic, Action, const std::string&);

	protected:
		virtual void invalid_state_transition_impl(
			client::State, const Message&);
		virtual void websocket_frame_too_big_impl(const Buffer& buffer);
		virtual void invalid_websocket_frame_flags_impl(int, int);
		virtual void sudden_disconnect_impl(const std::string& uri);
		virtual void timeout_impl(Topic, Action);
		virtual void timeout_impl(Topic, Action, const std::string&);
	};
}

#endif
