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

#include <cassert>
#include <cstdio>

#include <algorithm>
#include <stdexcept>
#include <map>

#include <deepstream/core/buffer.hpp>
#include <deepstream/core/event.hpp>
#include <deepstream/core/client.hpp>

#include "message_builder.hpp"

namespace deepstream {

Event::Event(const SendFn& send, SubscriptionId &subscription_counter)
    : send_(send)
    , send_queue_()
    , subscription_counter_(subscription_counter)
{
    assert(send_);
}

Event::~Event()
{
}

void Event::emit(const Name& name, const Buffer& buffer)
{
    if (name.empty())
        throw std::invalid_argument("Empty event name");

    MessageBuilder evt(Topic::EVENT, Action::EVENT);
    evt.add_argument(name);
    evt.add_argument(buffer);

    if (!send_(evt)) {
      // sending failed, or the connection is down
      send_queue_.emplace(new MessageBuilder(evt));
    }

    if (subscriber_map_.find(name) == subscriber_map_.end())
        return;

    notify_(evt);
}

SubscriptionId Event::subscribe(const Name& name, const SubscribeFn callback)
{
    if (name.empty()) {
        throw std::invalid_argument("Empty event subscription pattern");
    }

    const SubscriptionId subscription_id = subscription_counter_++;
    const auto insert_result =
        subscribe_fn_map_.insert(std::make_pair(subscription_id, callback));
    assert(insert_result.second);

    SubscriberList &subscribers = subscriber_map_[name];

    if (subscribers.empty()) {
        MessageBuilder message(Topic::EVENT, Action::SUBSCRIBE);
        message.add_argument(name);
        send_(message);
    }

    assert(std::find(subscribers.cbegin(), subscribers.cend(), subscription_id)
            == subscribers.end());
    subscribers.push_back(subscription_id);

    return subscription_id;
}

/**
 * Unsubscribe all callbacks for name
 */
void Event::unsubscribe(const Name& name)
{
    SubscriberMap::iterator sub_map_it = subscriber_map_.find(name);

    if (sub_map_it == subscriber_map_.end()) {
        // TODO: warn, subscription did not exist
        return;
    }

    assert(sub_map_it->first == name);
    SubscriberList &subscribers = sub_map_it->second;
    for (SubscriptionId id: subscribers) {
        const size_t removed = subscribe_fn_map_.erase(id);
        assert(removed == 1);
    }

    subscriber_map_.erase(sub_map_it);

    if (subscriber_map_.empty()) {
        MessageBuilder message(Topic::EVENT, Action::UNSUBSCRIBE);
        message.add_argument(name);
        send_(message);
    }
}

/**
 * Only unsubscribe the callback with the given SubscriptionId
 */
void Event::unsubscribe(const Name& name, const SubscriptionId subscription_id)
{
    const auto sub_map_it = subscriber_map_.find(name);

    if (sub_map_it == subscriber_map_.end())
        return;

    assert(sub_map_it->first == name);
    SubscriberList &subscribers = sub_map_it->second;

    const auto sub_it = std::find(subscribers.begin(), subscribers.end(), subscription_id);
    if (sub_it == subscribers.end()) {
        assert(subscribe_fn_map_.find(subscription_id) == subscribe_fn_map_.end());
        // TODO: warn, subscription did not exist
        return;
    }
    subscribers.erase(sub_it);

    // delete the callback
    const size_t removed = subscribe_fn_map_.erase(subscription_id);
    assert(removed == 1);

    if (subscribers.empty()) {
        unsubscribe(name);
    }
}

void Event::listen(const Name& pattern, const ListenFn callback)
{
    if (pattern.empty())
        throw std::invalid_argument("Cannot listen for empty patterns");

    auto listener_it = listener_map_.find(pattern);

    if (listener_it != listener_map_.end()) {
        // TODO: error, there is already a listener!
        return;
    }

    listener_map_[pattern] = callback;

    MessageBuilder message(Topic::EVENT, Action::LISTEN);
    message.add_argument(pattern);
    send_(message);
}

void Event::unlisten(const Name& pattern)
{
    auto listen_map_it = listener_map_.find(pattern);

    if (listen_map_it == listener_map_.end()) {
        //TODO: warn, not currently listening to the given pattern
        return;
    }

    listener_map_.erase(listen_map_it);

    MessageBuilder message(Topic::EVENT, Action::UNLISTEN);
    message.add_argument(pattern);
    send_(message);
}

void Event::notify_(const Message& message)
{
    assert(message.topic() == Topic::EVENT);

    switch (message.action()) {
    case Action::EVENT:
        notify_subscribers_(message);
        break;

    case Action::SUBSCRIPTION_FOR_PATTERN_FOUND:
    case Action::SUBSCRIPTION_FOR_PATTERN_REMOVED:
        assert(!message.is_ack());
        notify_listeners_(message);
        break;

    case Action::LISTEN:
    case Action::SUBSCRIBE:
        assert(message.is_ack());
        break;

    case Action::UNLISTEN:
    case Action::UNSUBSCRIBE:
        assert(message.is_ack());
        break;

    default:
        assert(0);
    }
}

void Event::notify_subscribers_(const Message& message)
{
    assert(message.action() == Action::EVENT);
    assert(message.num_arguments() == (message.is_ack() ? 1 : 2));

    if (message.is_ack())
        return;

    Name name = message[0];
    const Buffer& data = message[1];

    SubscriberMap::iterator it = subscriber_map_.find(name);

    if (it == subscriber_map_.end()) {
        name.push_back(0);

        std::fprintf(stderr, "E|EVT: no subscriber named '%s'\n", name.data());
        return;
    }

    assert(it->first == name);

    // Copying the list of subscribers is a necessity here because the callbacks
    // may unsubscribe during their execution and modifications of a subscriber
    // list may invalidate iterations and ranges.
    SubscriberList subscribers = it->second;

    // The iterator `it` may be invalid after executing a callback because the
    // list of subscribers for this `name` may have been erased.
    it = subscriber_map_.end();

    for (const SubscriptionId& id : subscribers) {
        const SubscribeFn &callback = subscribe_fn_map_[id];
        callback(data);
    }
}

void Event::notify_listeners_(const Message& message)
{
    assert(message.action() == Action::SUBSCRIPTION_FOR_PATTERN_FOUND || message.action() == Action::SUBSCRIPTION_FOR_PATTERN_REMOVED);
    assert(!message.is_ack());
    assert(message.num_arguments() == 2);

    const Name& pattern = message[0];
    const Name& match = message[1];

    bool is_subscribed = message.action() == Action::SUBSCRIPTION_FOR_PATTERN_FOUND;

    ListenerMap::iterator it = listener_map_.find(pattern);

    if (it == listener_map_.end()) {
        Name pattern_str(pattern);
        pattern_str.push_back(0);

        std::fprintf(stderr, "%s: no listener for pattern '%s'\n",
            message.header().to_string(), pattern_str.data());

        return;
    }

    assert(it->first == pattern);

    ListenFn callback = it->second;
    bool accept = callback(match, is_subscribed);

    if (message.action() == Action::SUBSCRIPTION_FOR_PATTERN_REMOVED)
        return;

    Action action = accept ? Action::LISTEN_ACCEPT : Action::LISTEN_REJECT;
    MessageBuilder el(Topic::EVENT, action);
    el.add_argument(pattern);
    el.add_argument(match);
    send_(el);
}

void Event::on_connection_state_change_(const ConnectionState state)
{
    if (state == ConnectionState::OPEN) {
        for (const auto &subscription: subscriber_map_) {
            const Name &subscriptionName = subscription.first;
            MessageBuilder message(Topic::EVENT, Action::SUBSCRIBE);
            message.add_argument(subscriptionName);
            if (!send_(message)) {
                break;
            }
        }
        for (const auto &listen: listener_map_) {
            const Name &pattern = listen.first;
            MessageBuilder message(Topic::EVENT, Action::LISTEN);
            message.add_argument(pattern);
            if (!send_(message)) {
                break;
            }
        }
        while (send_queue_.size() > 0) {
            const auto &message = *send_queue_.front();
            if (!send_(message)) {
                break;
            }
            send_queue_.pop();
        }
    }
}
}
