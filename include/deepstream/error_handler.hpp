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
#ifndef DEEPSTREAM_ERROR_HANDLER_HPP
#define DEEPSTREAM_ERROR_HANDLER_HPP

#include <string>
#include <system_error>

namespace deepstream {
enum class Topic;
enum class Action;

struct Buffer;
struct Message;

namespace client {
    enum class State;
}

namespace parser {
    struct Error;
}

namespace websockets {
    struct Frame;
}

/**
 * This class is an interface for the error handlers in the deepstream
 * client.
 *
 * It provides one virtual method for each error case. By default, the
 * methods print an error message to standard error.
 */
struct ErrorHandler {
    virtual ~ErrorHandler() = default;

    void parser_error(const parser::Error&);

    void invalid_state_transition(client::State, const Message&);

    /**
     * @param[in] num_redirections The number of redirections before calling
     * the error handler.
     */
    void too_many_redirections(unsigned num_redirections);

    void system_error(const std::system_error&);

    /**
     * A standard websocket close frame comes with a 2 byte payload
     * containing a <a
     * href="https://tools.ietf.org/html/rfc6455#section-7.4">status
     * code</a> in network byte order; this function handler is called if
     * the payload size is not two.
     */
    void invalid_close_frame_size(const websockets::Frame&);

    /**
     * This method handles exceptions thrown by the used websocket
     * implementation.
     */
    void websocket_exception(const std::exception&);

    /**
     * Currently, the implementation can only handle non-fragmented text and
     * close frames; this error handler is called if a different kind of
     * frame is received.
     */
    void unexpected_websocket_frame_flags(int got);

    /**
     * A socket read returned zero indicating the server disconnected (cf
     * Section *Return Values* in `man 2 read` or `man 2 recv`).
     */
    void sudden_disconnect(const std::string& uri);

    /**
     * This error handles is invoked when too many authentications attempts
     * failed, i.e., after receiving a `A|E|E_TOO_MANY_AUTH_ATTEMPTS|msg`
     * message.
     */
    void authentication_error(const Message& msg);

protected:
    virtual void parser_error_impl(const parser::Error&);

    virtual void invalid_state_transition_impl(client::State, const Message&);

    virtual void too_many_redirections_impl(unsigned);

    virtual void system_error_impl(const std::system_error&);

    virtual void invalid_close_frame_size_impl(const websockets::Frame&);

    virtual void websocket_exception_impl(const std::exception&);

    virtual void unexpected_websocket_frame_flags_impl(int);

    virtual void sudden_disconnect_impl(const std::string& uri);

    virtual void authentication_error_impl(const Message& msg);
};
}

#endif
