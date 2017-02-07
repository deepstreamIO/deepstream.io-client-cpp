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
#ifndef DEEPSTREAM_HPP
#define DEEPSTREAM_HPP

#include <cstdint>

#include <functional>
#include <memory>
#include <string>

#include <deepstream/buffer.hpp>
#include <deepstream/config.h>
#include <deepstream/client.hpp>
#include <deepstream/error_handler.hpp>
#include <deepstream/event.hpp>
// 201701227:
// we have to include `parser.hpp` because `parser::MessageList`,
// `parser::ErrorList` are `typedef`s and cannot be forward declared.
#include <deepstream/parser.hpp>
#include <deepstream/presence.hpp>
#include <deepstream/websockets.hpp>


namespace deepstream
{
	struct Message;
	struct ErrorHandler;

	namespace client
	{
		enum class State;
	}

	namespace websockets
	{
		enum class State;
		struct Client;
	}


	struct version
	{
		version() = delete;

		enum
		{
			MAJOR = DEEPSTREAM_VERSION_MAJOR,
			MINOR = DEEPSTREAM_VERSION_MINOR,
			PATCH = DEEPSTREAM_VERSION_PATCH
		};

		static constexpr const char* to_string() {
			return DEEPSTREAM_VERSION;
		}
	};


	struct Client
	{
		static std::string getUid();


		/**
		 * Given a *connected* websocket client and an error handler, construct
		 * a deepstream client.
		 *
		 * @post On exit, the client is either in `AWAIT_CONNECTION` or `CLOSED`
		 * state.
		 *
		 * @param[in] p_websocket A non-NULL pointer
		 * @param[in] p_error_handler A non-NULL pointer
		 *
		 */
		static Client make(
			std::unique_ptr<websockets::Client> p_websocket,
			std::unique_ptr<ErrorHandler> p_error_handler
		);
		static Client make(const std::string& uri);


		Client() = delete;
		Client(const Client&) = delete;
		Client(Client&&) = default;

	protected:
		explicit Client(
			std::unique_ptr<websockets::Client> p_websocket,
			std::unique_ptr<ErrorHandler>);

	public:
		/**
		 * Given a client in `AWAIT_CONNECTION` state, this function attempts to
		 * log in with the given authentication data.
		 *
		 * The format of the user authentication data depends on the <a
		 * href="https://deepstream.io/tutorials/core/security-overview/">authentication</a>
		 * method used by the server.
		 *
		 * @param[in] auth User authentication data
		 * @param[out] p_user_data On a successful reeturn, store the user data
		 * in `p_user_data` if the reference is not `NULL`.
		 *
		 * @return `AWAIT_CONNECTION`, `CLOSED`, or `CONNECTED`; `ERROR` if the
		 * client was not connected before
		 */
		client::State login(const std::string& auth, Buffer* p_user_data);
		void close();

		client::State getConnectionState() { return state_; }


		/**
		 * This function reads all incoming messages from the websocket and
		 * executes the appropriate callbacks; it returns when there are no
		 * messages left.
		 */
		void process_messages();


		Event event;
		Presence presence;


		/**
		 * This function reads messages from the websocket.
		 *
		 * @param[out] p_buffer A non-NULL pointer. After a successful exit, the
		 * buffer stores the unparsed messages.
		 * @param[out] p_messages A non-NULL pointer. After a successful exit,
		 * the messages reference the storage of `p_buffer`.
		 */
		websockets::State receive_(
			Buffer* p_buffer, parser::MessageList* p_messages
		);
		/**
		 * This method serializes the given messages and sends it as a
		 * non-fragmented text frame to the server.
		 */
		websockets::State send_(const Message&);

		/**
		 * This method sends a non-fragmented text frame with the contents of
		 * the given buffer as payload.
		 */
		websockets::State send_frame_(const Buffer&);
		/**
		 * This method sends a frame with given flags and the contents of the
		 * given buffer as payload.
		 */
		websockets::State send_frame_(const Buffer&, int);


		client::State state_;
		std::unique_ptr<websockets::Client> p_websocket_;

		std::unique_ptr<ErrorHandler> p_error_handler_;
	};
}

#endif
