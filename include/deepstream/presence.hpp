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
#ifndef DEEPSTREAM_PRESENCE_HPP
#define DEEPSTREAM_PRESENCE_HPP

#include <functional>
#include <memory>
#include <vector>

namespace deepstream {
struct Buffer;
struct Message;

struct Presence {
    typedef Buffer Name;

    /**
   * This alias is the signature of a deepstream present subscription
   * callback.
   */
    typedef std::function<void(const Name&, bool)> SubscribeFn;
    /**
     * The representation of a callback is stored as a smart pointer.
     *
     * Given an event name, the deepstream API allows the selective removal
     * of subscription callbacks. Hence, we need to be able to compare
     * functions. This is not possible `std::function` objects but with a
     * smart pointer we solve two problems:
     * - we can compare function references for equality,
     * - if a callback removes itself as a callback, we can ensure that the
     *   `std::function` destructor is not called before returning.
     */
    typedef std::shared_ptr<SubscribeFn> SubscribeFnPtr;
    typedef std::vector<SubscribeFnPtr> SubscriberList;

    typedef std::vector<Name> UserList;
    /**
     * This alias is the signature of a deepstream callback for presence
     * queries.
     */
    typedef std::function<void(const UserList&)> QueryFn;
    // "querent" is actually an obsolete word:
    //   https://en.wiktionary.org/wiki/querent
    // we do not need smart pointers here (like for the subscriber list)
    // because query functions cannot remove from this list.
    typedef std::vector<QueryFn> QuerentList;

    /**
     * This alias is the signature of the function used to send messages to
     * the deepstream server.
     *
     * Function matching this signature are expected to return `true` if the
     * message was sent successfully, `false` otherwise. As of Feb 6, 2017,
     * the return value is unused.
     */
    typedef std::function<bool(const Message&)> SendFn;

    /**
     * The constructor takes a function that sends a message to the
     * deepstream server.
     *
     * With this constructor instead of `Presence(deepstream::Client*)` it
     * becomes easier to test this module.
     */
    explicit Presence(const SendFn&);

    /**
     * This method registers the given function as callback for presence
     * notifications.
     *
     * @return A smart pointer which can be used to unsubscribe
     */
    SubscribeFnPtr subscribe(const SubscribeFn&);

    void subscribe(const SubscribeFnPtr&);

    /**
     * This function removes the given callback from the list of presence
     * subscribers.
     */
    void unsubscribe(const SubscribeFnPtr&);

    /**
     * This function queries the server for a list of all logged in users.
     *
     * Note that clients with anonymous log-ins are not in the list.
     * Similarly, multiple clients can log-in as the same user but the user
     * will be listed only once.
     */
    // Christoph, Feb 7, 2017:
    // this name is in camelCase in the JS API but I was using pascal_case
    // throughout the code for functions so I decided to use pascal_case
    // here, too.
    void get_all(const QueryFn&);

    /**
     * This method handles resence-related messages (messages with topic
     * `Topic::PRESENCE`) from the server.
     */
    void notify_(const Message&);

    SendFn send_;
    SubscriberList subscribers_;
    QuerentList querents_;
};
}

#endif
