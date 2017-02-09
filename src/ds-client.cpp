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

#include <deepstream.hpp>

int main()
try
{
	deepstream::Client client =
		deepstream::Client::make("ws://localhost:6020/deepstream");
	if( client.login() )
		std::printf( "Client logged in\n" );
	else
		std::printf( "Client not logged in\n" );
}
catch(std::exception& e)
{
	fprintf( stderr, "error: '%s'\n", e.what() );
	return 1;
}
