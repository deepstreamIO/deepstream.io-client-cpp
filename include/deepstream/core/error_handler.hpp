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

namespace deepstream {

enum class ErrorState {
    PARSER_ERROR,
    INVALID_STATE_TRANSITION,
    TOO_MANY_REDIRECTIONS,
    SYSTEM_ERROR,
    INVALID_CLOSE_FRAME_SIZE,
    WEBSOCKET_EXCEPTION,
    UNEXPECTED_WEBSOCKET_FRAME_FLAGS,
    SUDDEN_DISCONNECT,
    AUTHENTICATION_ERROR,
};

/**
 * This class is an interface for the error handlers in the deepstream
 * client.
 */
struct ErrorHandler {
    virtual ~ErrorHandler() = default;
    virtual void on_error(const std::string &) const {};
};
}

#endif
