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
#include <string>

#include <deepstream/buffer.hpp>
#include <deepstream/config.h>
#include <deepstream/client.hpp>
#include <deepstream/event.hpp>
#include <deepstream/presence.hpp>


namespace deepstream
{
	struct ErrorHandler;

	namespace client
	{
		enum class State;
	}

	namespace impl
	{
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


	// This class does not use `std::unique_ptr<impl::Client>` because it
	// prevents the use of the PIMPL idiom because the class attempts to
	// evaluate `sizeof(impl::Client)`; naturally, we must know the definition
	// of `impl::Client` meaning we have to include the appropriate header.
	struct Client
	{
		static std::string getUid();


		Client(const std::string& uri);
		Client(const std::string& uri, std::unique_ptr<ErrorHandler>);
		~Client();

		Client() = delete;
		Client(const Client&) = delete;
		Client(Client&&) = default;

	public:
		/**
		 * Given a client in `AWAIT_CONNECTION` state, this function attempts to
		 * log in anonymously.
		 *
		 * @return `true` if the log in attempt was successful, `false`
		 * otherwise
		 */
		bool login();
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
		 * @return `true` if the log in attempt was successful, `false`
		 * otherwise
		 */
		bool login(const std::string& auth, Buffer* p_user_data=nullptr);
		void close();

		client::State getConnectionState();


		/**
		 * This function reads all incoming messages from the websocket and
		 * executes the appropriate callbacks; it returns when there are no
		 * messages left.
		 */
		void process_messages();


		impl::Client* const p_impl_;
		Event event;
		Presence presence;
	};
}

#endif
