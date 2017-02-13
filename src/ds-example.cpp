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
#include <exception>
#include <iostream>

#include <deepstream.hpp>

int main()
try
{
	using deepstream::Buffer;

	deepstream::Client client("ws://localhost:6020/deepstream");
	if( !client.login() ) {
		std::cout << "Client not logged in" << std::endl;
		return 1;
	}
	std::cout << "Client logged in" << std::endl;

	using deepstream::Event;

	// Subscribe to the event "adam"
	Event::SubscribeFnPtr sub_ptr
		= client.event.subscribe(Buffer("adam"), [&](const Buffer& buff){
		// print the event data
		std::string buff_str(buff.begin(), buff.end());
		std::cout << buff_str << std::endl;
		// emit the "eve" event
		client.event.emit(Buffer("eve"), Buffer("Sbar"));
		// unsubscribe from the "adam" event
		client.event.unsubscribe(Buffer("adam"), sub_ptr);
	});

	// Listen for subscriptions to events beginning with "foobar"
	client.event.listen(Buffer("foobar.*"), [](const Buffer& match, bool isSubscribed){
		std::string match_str(match.begin(), match.end());
		if (isSubscribed) {
			std::cout << "someone is listening to event " << match_str << std::endl;
		} else {
			std::cout << "nobody is listening to event " << match_str << std::endl;
		}
		// accept the listen
		return true;
	});

	using deepstream::Presence;

	// List all present users
	// FIXME: "Sudden disconnect" errors
/*
 *    client.presence.get_all([](const Presence::UserList users){
 *        std::cout << "Users: " << std::endl;
 *
 *        for (auto user_buff : users) {
 *            std::string user_str(user_buff.begin(), user_buff.end());
 *            std::cout << "\t" << user_str << std::endl;
 *        }
 *    });
 */

	while(true) {
		client.process_messages();
	}
}
catch(std::exception& e)
{
	std::cerr << "error: '" << e.what() << "'" << std::endl;
	return 1;
}
