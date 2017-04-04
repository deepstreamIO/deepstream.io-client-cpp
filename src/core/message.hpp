/*
 * Copyright 2016-2017 deepstreamHub GmbH
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
#ifndef DEEPSTREAM_MESSAGE_HPP
#define DEEPSTREAM_MESSAGE_HPP

#include <cstddef>

#include <iosfwd>
#include <utility>

namespace deepstream {
struct Buffer;

const char ASCII_RECORD_SEPARATOR = 30;
const char ASCII_UNIT_SEPARATOR = 31;

enum class Topic { AUTH,
    CONNECTION,
    ERROR,
    EVENT,
    PRESENCE,
    RECORD,
    RPC };

enum class Action {
    CHALLENGE,
    CHALLENGE_RESPONSE,
    CREATE_OR_READ,
    ERROR_INVALID_AUTH_DATA,
    ERROR_INVALID_AUTH_MSG,
    ERROR_TOO_MANY_AUTH_ATTEMPTS,
    EVENT,
    LISTEN,
    LISTEN_ACCEPT,
    LISTEN_REJECT,
    PING,
    PONG,
    PRESENCE_JOIN,
    PRESENCE_LEAVE,
    QUERY,
    REDIRECT,
    REJECT,
    REQUEST,
    SUBSCRIBE,
    SUBSCRIPTION_FOR_PATTERN_FOUND,
    SUBSCRIPTION_FOR_PATTERN_REMOVED,
    UNLISTEN,
    UNSUBSCRIBE
};

enum class Sender { CLIENT,
    SERVER };

// needed by Boost.Test checks
std::ostream& operator<<(std::ostream&, Topic);

std::ostream& operator<<(std::ostream&, Action);

std::ostream& operator<<(std::ostream&, Sender);

/**
 * This is an interface for different message representations providing
 * access to the data contained within a message (header and payload).
 *
 * This class uses the Non-Virtual Interface Idiom:
 * http://www.gotw.ca/publications/mill18.htm
 */
struct Message {
    struct Header {
        /**
         * This function returns the list of all valid message headers.
         *
         * The return value is a pair of iterators forming a half-open
         * interval.
         */
        static std::pair<const Header*, const Header*> all();

        /**
         * This function returns the human-readable representation of the
         * given message header.
         *
         * The input must form a valid message header.
         */
        static const char* to_string(Topic, Action, bool is_ack = false);

        /**
         * This function returns the number of bytes needed to store the
         * serialized message header in a deepstream message or its
         * human-readable representation.
         */
        static std::size_t size(Topic, Action, bool is_ack = false);

        explicit Header(Topic topic, Action action, bool is_ack = false)
            : topic_impl_(topic)
            , action_impl_(action)
            , is_ack_impl_(is_ack)
        {
        }

        /**
         * @return The human-readable representation of the header
         */
        const char* to_string() const;

        /**
         * @return The length of the human-readable header representation in
         * bytes
         */
        std::size_t size() const;

        /**
         * This method returns the representation of this header in a
         * deepstream message.
         */
        Buffer to_binary() const;

        Topic topic() const { return topic_impl_; }

        Action action() const { return action_impl_; }

        bool is_ack() const { return is_ack_impl_; }

        Topic topic_impl_;
        Action action_impl_;
        bool is_ack_impl_;
    };

    /**
     * This function takes a human-readable deepstream message, e.g.,
     * `E|A|S|event+`, and returns its machine-readable counterpart by
     * replacing `|` with the ASCII character 31 (unit separator) and `+`
     * with ASCII character 30 (record separator).
     *
     * @param[in] p A null-terminated string
     */
    static Buffer from_human_readable(const char* p);

    static std::string to_human_readable(const Buffer &buff);

    /**
     * @param[in] p A pointer to a string of length `size`
     * @param[in] size The length of the string referenced by p
     */
    static Buffer from_human_readable(const char* p, std::size_t size);

    /**
     * This function returns the minimum and maximum number of arguments
     * for every message, e.g., for "E|S|event+" (event subscription), this
     * function returns the pair (1, 1).
     */
    static std::pair<std::size_t, std::size_t> num_arguments(const Header&);

    virtual ~Message() = default;

    /**
     * This method returns the size of the message in bytes.
     */
    std::size_t size() const { return size_impl_(); }

    const Header& header() const { return header_impl_(); }

    Topic topic() const { return header().topic(); }

    Action action() const { return header().action(); }

    bool is_ack() const { return header().is_ack(); }

    std::size_t num_arguments() const { return num_arguments_impl_(); }

    /**
     * This operator returns the i-th argument of a message.
     */
    Buffer operator[](std::size_t) const;

    /**
     * This method returns the assembled deepstream message.
     */
    Buffer to_binary() const;

    virtual std::size_t size_impl_() const = 0;

    virtual const Header& header_impl_() const = 0;

    virtual std::size_t num_arguments_impl_() const = 0;

    virtual Buffer get_impl_(std::size_t) const = 0;

    virtual Buffer to_binary_impl_() const = 0;
};

std::ostream& operator<<(std::ostream&, const Message::Header&);

bool operator==(const Message::Header&, const Message::Header&);
}

#endif
