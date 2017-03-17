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

#pragma once

#include <functional>
#include <string>

// Note: send() and recv() semantics are copied from
// Poco::Net:WebSocket.

namespace deepstream {
class WS {
public:
    typedef std::function<void(const std::string&)> HandlerWithMsgFn;
    typedef std::function<void()> HandlerFn;

    WS(){};

    virtual ~WS(){};

    virtual std::string URI() const = 0;

    // Sends the contents of the given buffer through the socket as a
    // single frame.
    //
    // The frame should always be sent as FRAME_TEXT.
    //
    // Returns the number of bytes sent, which may be less than the
    // number of bytes specified.
    //
    // Certain socket implementations may also return a negative value
    // denoting a certain condition.
    virtual int send(const void* buffer, int length) const = 0;

    // Receives a frame from the socket and stores it in buffer. Up to
    // length bytes are received. If the frame's payload is larger, a
    // WebSocketException is thrown and the WebSocket connection must
    // be terminated.
    //
    // Returns the number of bytes received. A return value of 0 means
    // that the peer has shut down or closed the connection.
    //
    // Throws a TimeoutException if a receive timeout has been set and
    // nothing is received within that interval. Throws a NetException
    // (or a subclass) in case of other errors.
    //
    // The frame flags and opcode (FrameFlags and FrameOpcodes)
    // are stored in flags.
    virtual int recv(void* buffer, int length, int& flags) const = 0;

    // Returns the number of bytes available that can be read without
    // causing the socket to block.
    //
    // For an SSL connection, returns the number of bytes that can be
    // read from the currently buffered SSL record, before a new
    // record is read from the underlying socket.
    virtual int available() const = 0;

    virtual bool open() = 0;

    virtual void close() = 0;

    virtual void shutdown() = 0;

    virtual void onClose(const HandlerFn&) const = 0;

    virtual void onError(const HandlerWithMsgFn&) const = 0;

    virtual void onMessage(const HandlerWithMsgFn&) const = 0;

    virtual void onOpen(const HandlerFn&) const = 0;

protected:
    WS(WS const&);

    WS& operator=(WS const&);
};

struct WSFactory {
    virtual WS* connect(const std::string& uri) = 0;
    virtual ~WSFactory() {}
};
}
