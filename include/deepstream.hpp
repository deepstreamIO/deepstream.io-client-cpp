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

// 201701227:
// we have to include `parser.hpp` because `parser::MessageList`,
// `parser::ErrorList` are `typedef`s and cannot be forward declared.
#include <parser.hpp>


namespace deepstream
{
	struct Buffer;
	struct Client;
	struct Message;
	struct ErrorHandler;

	namespace client
	{
		enum class State;
	}

	namespace websockets
	{
		enum class StatusCode;
		enum class State;
		struct Client;
	}


	struct Event
	{
		explicit Event(Client* p_impl);

		void subscribe(
			const Buffer&,
			const std::function<void(const Buffer&, const Buffer&)>
		);

		void unsubscribe(const Buffer&);
		void unsubscribe(
			const Buffer&,
			const std::function<void(const Buffer&, const Buffer&)>
		);

		template<typename T>
		void emit(const Buffer&, const T& t);

		void listen(
			const std::string& pattern,
			std::function<void(const std::string&, bool, const Buffer&)>
		);

		void unlisten(const std::string& pattern);


		Client* p_impl_;
	};


	struct Presence
	{
		explicit Presence(Client* p_impl);

		void subscribe(std::function<void(const std::string&, bool)>);
		void unsubscribe(std::function<void(const std::string&, bool)>);

		void getAll(std::function<void(const std::vector<Buffer>&)>);


		Client* p_impl_;
	};


	struct RecordFactory
	{
	};


	struct RemoteProcedureCall
	{
		enum class Error;

		struct Response
		{
			explicit Response(Client* p_impl);

			void send(const Buffer&);
			void reject();
			void error(const std::string&);
			void ack();
		};


		explicit RemoteProcedureCall(Client* p_impl);

		void provide(
			const std::string&,
			std::function<void(const Buffer& data, Response&)>
		);

		void unprovide(const std::string&);

		void make(
			const std::string&,
			const Buffer&,
			std::function<void(Error, const Message&)>
		);
	};


	struct Client
	{
		static std::string getUid();


		static Client make(
			std::unique_ptr<websockets::Client>,
			std::unique_ptr<ErrorHandler>
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
		client::State login(const std::string& auth, Buffer* p_user_data);
		void close();

		client::State getConnectionState() { return state_; }


		websockets::State receive_(
			Buffer* p_buffer, parser::MessageList* p_messages
		);
		websockets::State send_(const Message&);


		client::State state_;
		std::unique_ptr<websockets::Client> p_websocket_;

		std::unique_ptr<ErrorHandler> p_error_handler_;
	};
}

#endif
