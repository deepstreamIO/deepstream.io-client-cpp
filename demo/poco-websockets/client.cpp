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
#include <cstddef>
#include <cstdio>

#include <algorithm>
#include <vector>

#include <arpa/inet.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/WebSocket.h>

#include <cassert>

namespace PN = Poco::Net;

typedef std::vector<char> Buffer;

Buffer from_human_readable(const char* p)
{
    std::size_t size = std::strlen(p);

    Buffer xs(p, p + size);
    std::replace(xs.begin(), xs.end(), '|', '\x1f');
    std::replace(xs.begin(), xs.end(), '+', '\x1e');

    return xs;
}

int main()
{
    PN::HTTPClientSession session("localhost", 6020);
    PN::HTTPResponse response;
    PN::HTTPRequest request(PN::HTTPRequest::HTTP_GET, "/deepstream",
        PN::HTTPRequest::HTTP_1_1);

    PN::WebSocket ws(session, request, response);

    char buffer[160];
    const int sz = sizeof(buffer);
    int flags = 0;
    int ret = 0;

    std::fill(buffer, buffer + sz, 0);
    flags = 0;
    ret = ws.receiveFrame(buffer, sz - 1, flags);

    std::printf("%3d %3d '%s'\n", ret, flags, buffer);

    const char CHR[] = { 'C', 31, 'C', 'H', 'R', 31, 'U', 'R', 'L', 30, 0 };
    const std::size_t CHR_SZ = std::strlen(CHR);

    ret = ws.sendFrame(CHR, CHR_SZ);
    std::printf("%3d\n", ret);

    std::fill(buffer, buffer + sz, 0);
    flags = 0;
    ret = ws.receiveFrame(buffer, sz - 1, flags);

    std::printf("%3d %3d '%s'\n", ret, flags, buffer);

    char AREQ[] = "A|REQ|{\"username\":\"a\",\"password\":\"b\"}+";
    auto areq = from_human_readable(AREQ);

    ret = ws.sendFrame(areq.data(), areq.size());
    std::printf("%3d\n", ret);

    std::fill(buffer, buffer + sz, 0);
    flags = 0;
    ret = ws.receiveFrame(buffer, sz - 1, flags);

    std::printf("%3d %3d '%s'\n", ret, flags, buffer);

    ws.shutdown();

    std::fill(buffer, buffer + sz, 0);
    flags = 0;
    ret = ws.receiveFrame(buffer, sz - 1, flags);

    std::printf("%3d %3d\n", ret, flags);

    int ws_EOF = PN::WebSocket::FRAME_FLAG_FIN | PN::WebSocket::FRAME_OP_CLOSE;

    assert(flags == ws_EOF);

    unsigned short* p_status_code = reinterpret_cast<unsigned short*>(buffer);
    assert(*p_status_code == htons(1000));
}
