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
#include <cstdio>

#include <exception>

#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>

#include <client.hpp>
#include <deepstream.hpp>
#include <websockets.hpp>
#include <error_handler.hpp>

#include <cassert>


namespace ds = deepstream;



int main()
try
{
	auto p_client = ds::Client::make("ws://localhost:6020/deepstream");


	const char AUTH[] =
		"{\"username\": \"user\", \"password\": \"pwd\"}";

	deepstream::Buffer user_data;
	while(true)
	{
		deepstream::client::State state = p_client->login( AUTH, &user_data );

		if( state == deepstream::client::State::CONNECTED )
			break;
	}
}
catch(Poco::Net::WebSocketException& e)
{
	const std::string& msg = e.message();

	fprintf(
		stderr,
		"poco websocket exception: '%s' (code=%d)\n",
		msg.c_str(), e.code()
	);
	return 1;
}
catch(Poco::Exception& e)
{
	const std::string& msg = e.message();

	fprintf(
		stderr,
		"poco exception: '%s' (code=%d)\n",
		msg.c_str(), e.code()
	);
	return 1;
}
catch(std::exception& e)
{
	fprintf( stderr, "error: '%s'\n", e.what() );
	return 1;
}
