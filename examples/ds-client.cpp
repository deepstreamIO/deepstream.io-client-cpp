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

#include <unistd.h> // usleep

#include <exception>
#include <iostream>

#include <deepstream/core.hpp>
#include <deepstream/lib/poco-ws.hpp>
#include <deepstream/lib/basic-error-handler.hpp>

int main(int argc, char* argv[])
{
    std::string uri = "ws://localhost:6020/deepstream";

    if (argc >= 2) {
        uri = argv[1];
    }

    bool done = false;
    try {
        deepstream::BasicErrorHandler errh;
        deepstream::PocoWSHandler wsh;
        deepstream::Client client(uri, wsh, errh);
        client.login("{}", [&](const std::unique_ptr<deepstream::Buffer> &){
                done = true;
            });

        while (true) {
            wsh.process_messages();
            if (client.get_connection_state() == deepstream::ConnectionState::CLOSED
                    || client.get_connection_state() == deepstream::ConnectionState::ERROR) {
                std::cerr << "failed to login to " << uri << std::endl;
                return EXIT_FAILURE;
            }
            if (done && client.get_connection_state() == deepstream::ConnectionState::OPEN) {
                std::cout << "successfully logged in to " << uri << std::endl;
                return EXIT_SUCCESS;
            }
            usleep(10e3);
        }
    } catch (std::exception& e) {
        std::cerr << "Caught exception \"" << e.what() << "\"" << std::endl;
        return EXIT_FAILURE;
    }
}
