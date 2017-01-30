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
#ifndef DEEPSTREAM_WEBSOCKETS_POCO_HPP
#define DEEPSTREAM_WEBSOCKETS_POCO_HPP

#include <string>
#include <utility>

#include <Poco/URI.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/WebSocket.h>

#include <time.hpp>


namespace deepstream
{
	struct Buffer;

	namespace websockets
	{
		enum class StatusCode;
		struct Frame;
		struct Client;

		namespace poco
		{
			struct Client : public ::deepstream::websockets::Client
			{
				explicit Client(const std::string& uri);


				virtual std::string uri_impl() const override;

				virtual std::size_t num_bytes_available_impl() override;

				virtual time::Duration get_receive_timeout_impl() override;
				virtual void set_receive_timeout_impl(time::Duration) override;

				virtual std::pair<StatusCode, std::unique_ptr<Frame> >
					receive_frame_impl() override;
				virtual int send_frame_impl(
					const Buffer& buffer, Frame::Flags
				) override;

				virtual void close_impl() override;


				Poco::URI uri_;
				Poco::Net::HTTPClientSession session_;
				Poco::Net::HTTPRequest request_;
				Poco::Net::HTTPResponse response_;
				Poco::Net::WebSocket websocket_;
			};
		}
	}
}

#endif
