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
#include <deepstream/json.hpp>

using deepstream::Buffer;
using deepstream::Event;
using deepstream::Presence;

using json = nlohmann::json;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: <URI>" << std::endl;
        return EXIT_FAILURE;
    }

    deepstream::Client client(argv[1]);

    json::object_t auth{{ "name", "bob" }};

    if (!client.login(auth)) {
        std::cout << "Client not logged in" << std::endl;
        return 1;
    }

    std::cout << "Client logged in" << std::endl;

    // Subscribe to the event "adam"
    Event::SubscribeFnPtr event_sub_ptr = client.event.subscribe(Buffer("adam"), [&](const Buffer& buff) {

        // print the event data
        std::string buff_str(buff.begin(), buff.end());
        std::cout << buff_str << std::endl;
        // emit the "eve" event
        client.event.emit(Buffer("eve"), Buffer("Sbar"));

        json j = {
            { "pi", 3.141 },
            { "happy", true },
            { "name", "json" },
            { "nothing", nullptr },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } }
        };

        // emit the "json" event
        client.event.emit(Buffer("json"), Buffer(j.dump().c_str()));

	//std::cout << "Sent JSON event:\n" << j.dump(4), "\n";

        // unsubscribe from the "adam" event
        // client.event.unsubscribe(Buffer("adam"), event_sub_ptr);
    });

    // Listen for subscriptions to events beginning with "foobar"
    client.event.listen(Buffer("foobar.*"), [](const Buffer& match, bool isSubscribed) {
        std::string match_str(match.begin(), match.end());
        if (isSubscribed) {
            std::cout << "someone is listening to event " << match_str << std::endl;
        } else {
            std::cout << "nobody is listening to event " << match_str << std::endl;
        }
        // accept the listen
        return true;
    });

    // Presence subscription allows clients to know when other clients
    // come online and offline.

    Presence::SubscribeFnPtr presence_sub_ptr = client.presence.subscribe([&](const Presence::Name& name, bool online) {
        std::string buff_str(name.begin(), name.end());
        std::cout << buff_str;
        std::cout << (online ? " is online" : " is offline") << std::endl;
        client.presence.unsubscribe(presence_sub_ptr);
    });

    client.presence.get_all([](const Presence::UserList users) {
        std::cout << "Users: " << std::endl;

        for (auto user_buff : users) {
            std::string user_str(user_buff.begin(), user_buff.end());
            std::cout << "\t" << user_str << std::endl;
        }
    });

    while (true) {
        client.process_messages();
    }
}
