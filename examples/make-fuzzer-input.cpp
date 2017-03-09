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
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

#include <stdexcept>

#include <string>
#include <vector>

#include <deepstream/buffer.hpp>
#include "../src/message_builder.hpp"
#include "../src/random.hpp"

#include <cassert>

using namespace deepstream;

unsigned long str2ul(const char* p)
{
    const int base = 10;
    const char* const end = p + std::strlen(p);

    char* q = nullptr;
    unsigned long ul = std::strtoul(p, &q, base);

    if (q != end) {
        std::string e("Input is not an unsigned long: ");
        throw std::invalid_argument(e + p);
    }

    return ul;
}

int main(int argc, char** argv)
{
    if (argc >= 4) {
        const char* aout = (argc > 0) ? argv[0] : "a.out";
        std::fprintf(stderr, "usage: %s [seed] [max num bytes]\n", aout);
        return EXIT_FAILURE;
    }

    random::Engine::result_type seed = 1;
    std::size_t max_num_bytes = 1000;

    try {
        if (argc >= 2)
            seed = str2ul(argv[1]);
        if (argc >= 3)
            max_num_bytes = str2ul(argv[2]);
    } catch (std::invalid_argument& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    std::vector<char> output;
    output.reserve(max_num_bytes);
    random::Engine engine(seed);

    while (true) {
        MessageBuilder m = random::make_message(&engine);

        if (output.size() + m.size() > max_num_bytes)
            break;

        Buffer binary = m.to_binary();
        output.insert(output.end(), binary.cbegin(), binary.cend());
    }

    int ret = write(STDOUT_FILENO, output.data(), output.size());

    if (ret < 0 || std::size_t(ret) != output.size()) {
        std::perror("write");
        return EXIT_FAILURE;
    }
}
