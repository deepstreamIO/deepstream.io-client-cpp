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
#ifndef DEEPSTREAM_WEBSOCKETS_PSEUDO_HPP
#define DEEPSTREAM_WEBSOCKETS_PSEUDO_HPP

#include <memory>
#include <string>
#include <utility>

#include <deepstream/time.hpp>


namespace deepstream
{
	struct Buffer;

	namespace websockets
	{
		enum class StatusCode;
		struct Frame;
		struct Client;

		namespace pseudo
		{
			struct Client : public ::deepstream::websockets::Client
			{
				explicit Client();


				virtual std::string uri_impl() const override;

				virtual time::Duration get_receive_timeout_impl() override;
				virtual void set_receive_timeout_impl(time::Duration) override;

				virtual void close_impl() override;


				time::Duration timeout_;
			};
		}
	}
}

#endif
