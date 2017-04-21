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
#ifndef DEEPSTREAM_EVENT_HPP
#define DEEPSTREAM_EVENT_HPP

#include <deepstream/core/fwd.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <queue>

namespace deepstream {

struct Event {
    typedef Buffer Name;

    /**
     * This alias is the signature of a deepstream event subscription
     * callback.
     */
    typedef std::function<void(const Buffer&)> SubscribeFn;
    /**
     * Subscriptions are identified by a unique id returned by the subscribe()
     * function
     *
     * Given an event name, the deepstream API allows the selective removal
     * of subscription callbacks by providing the given identifier in calls to
     * unsubscribe.
     */
    typedef std::vector<SubscriptionId> SubscriberList;
    typedef std::map<SubscriptionId, SubscribeFn> SubscribeFnMap;
    typedef std::map<Name, SubscriberList> SubscriberMap;

    /**
     * The following alias is the signature of a deepstream event listener
     * callback.
     */
    typedef std::function<bool(const Name&, bool)> ListenFn;
    /**
     * The representation of a callback is stored as a smart pointer.
     */
    typedef std::map<Name, ListenFn> ListenerMap;

    /**
     * This alias is the signature of the function used to send messages to
     * the deepstream server.
     *
     * Function matching this signature are expected to return `true` if the
     * message was sent successfully, `false` otherwise.
     */
    typedef std::function<bool(const Message&)> SendFn;

    /**
     * The constructor takes a function that sends a message to the
     * deepstream server.
     *
     * With this constructor instead of `Event(deepstream::Client*)` it
     * becomes easier to test this module.
     */
    explicit Event(const SendFn &, SubscriptionId &);

    ~Event();

    Event() = delete;

    Event(const Event &) = delete;
    Event &operator=(const Event &) = delete;

    /**
     * This function emits an event with the provided name and the given
     * buffer as its data.
     */
    void emit(const Name&, const Buffer&);

    /**
     * This method subscribes the given function to the event with the given
     * name.
     *
     * @return A SubscriptionId which can be used to unsubscribe
     */
    SubscriptionId subscribe(const Name&, const SubscribeFn);

    /**
     * This function unsubscribes *all* callbacks from the given event.
     */
    void unsubscribe(const Name&);

    /**
     * This function removes the given callback from the list of subscribers
     * for the given event.
     */
    void unsubscribe(const Name&, const SubscriptionId);

    /**
     * This method adds a callback that listens for subscriptions matching
     * the given pattern.
     *
     * Existing listeners are not overwritten.
     */
    void listen(const Name& pattern, const ListenFn);

    /**
     * Stop listening to the given pattern
     */
    void unlisten(const Name& pattern);

    /**
     * This method is called when event-related messages arrive (messages
     * with topic `Topic::EVENT`) from the server.
     */
    void notify_(const Message&);

    /**
     * This method handles messages from the server related to event
     * subscription.
     */
    void notify_subscribers_(const Message&);

    /**
     * This method handles messages from the server related to event
     * listening.
     */
    void notify_listeners_(const Message&);

    void on_connection_state_change_(const ConnectionState);

    const SendFn send_;
    SubscriberMap subscriber_map_;
    SubscribeFnMap subscribe_fn_map_;
    ListenerMap listener_map_;

  private:

    void send_buffered(const std::unique_ptr<Message> &&);

    std::queue<std::unique_ptr<Message>> send_queue_;

    SubscriptionId &subscription_counter_;
};
}

#endif
