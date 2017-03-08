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

#include <cstdlib>
#include <exception>
#include <iostream>

#include <deepstream.hpp>
#include <deepstream/json.hpp>

using json = nlohmann::json;

int main(int argc, char* argv[])
{
    std::string uri = "ws://localhost:6020/deepstream";

    if (argc >= 2) {
        uri = argv[1];
    }

    try {
        deepstream::Client client(uri);

	json::object_t auth{{"username", ::getenv("USER")}};

        if (client.login(auth)) {
            std::cout << "successfully logged in to " << uri << std::endl;
            return EXIT_SUCCESS;
        } else {
            std::cerr << "failed to login to " << uri << std::endl;
            return EXIT_FAILURE;
        }
    } catch (std::exception& e) {
        std::cerr << "Caught exception \"" << e.what() << "\"" << std::endl;
        return EXIT_FAILURE;
    }
}
