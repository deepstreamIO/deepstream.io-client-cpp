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

#include <vector>

#include <deepstream/parser.hpp>

#include <cassert>

using namespace deepstream;

int main(int argc, char** argv)
{
    if (argc != 1) {
        const char* aout = (argc > 0) ? argv[0] : "a.out";
        std::fprintf(stderr, "%s reads only from standard input\n", aout);
        return EXIT_FAILURE;
    }

    while (true) {
        std::vector<char> buffer(4096, 0);

        int ret = read(STDIN_FILENO, buffer.data(), buffer.size() - 2);

        if (ret == 0)
            return 0;

        if (ret < 0) {
            std::perror("read");
            return EXIT_FAILURE;
        }

        parser::execute(buffer.data(), ret + 2);
    }
}
