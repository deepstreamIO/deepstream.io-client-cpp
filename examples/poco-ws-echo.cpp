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

#include <deepstream/core.hpp>
#include <deepstream/lib/poco-ws.hpp>
#include <deepstream/lib/basic-error-handler.hpp>

int main(int argc, char* argv[])
{
    std::string uri = "ws://echo.websocket.org/";

    if (argc >= 2) {
        uri = argv[1];
    }

    bool done = false;
    try {
        deepstream::PocoWSHandler wsh;
        wsh.URI(uri);
        wsh.on_open([](){
                std::cout << "OPEN" << std::endl;
                });
        wsh.on_close([&done](){
                std::cout << "CLOSE" << std::endl;
                done = true;
                });
        wsh.on_error([](const std::string &&error){
                std::cout << "ERROR: " << error << std::endl;
                });
        wsh.on_message([](const deepstream::Buffer &&message){
                std::string message_str(message.cbegin(), message.cend());
                std::cout << "MESSAGE: " << message_str << std::endl;
                });
        wsh.open();

        do {
            std::string input;
            std::cin >> input;
            deepstream::Buffer input_buff(input.cbegin(), input.cend());
            if (input == "q") {
                wsh.close();
            } else {
                wsh.send(input_buff);
            }
            usleep(400e3);
            wsh.process_messages();
            wsh.process_messages();
        } while (!done);
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
