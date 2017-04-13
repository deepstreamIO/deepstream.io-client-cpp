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

#include <algorithm>
#include <stdexcept>

#include <deepstream/core/buffer.hpp>
#include "message_builder.hpp"
#include <deepstream/core/presence.hpp>

#include <cassert>

namespace deepstream {

Presence::Presence(const SendFn& send, SubscriptionId &subscription_counter)
    : send_(send)
    , subscription_counter_(subscription_counter)
{
    assert(send_);
}

SubscriptionId Presence::subscribe(const SubscribeFn callback)
{
    const SubscriptionId subscription_id = subscription_counter_++;

    const auto insert_result =
        subscribe_fn_map_.insert(std::make_pair(subscription_id, callback));
    const bool insert_success = insert_result.second;
    assert(insert_success);

    if (subscribers_.empty()) {
        MessageBuilder presence_subscribe(Topic::PRESENCE, Action::SUBSCRIBE);
        send_(presence_subscribe);
    }

    subscribers_.push_back(subscription_id);

    return subscription_id;
}

void Presence::unsubscribe(const SubscriptionId subscription_id)
{
    assert(subscribe_fn_map_.size() == subscribers_.size());

    auto sub_it = std::find(subscribers_.begin(), subscribers_.end(), subscription_id);

    if (sub_it == subscribers_.end()) {
        // TODO: warn, subscription did not exist
        return;
    }

    subscribers_.erase(sub_it);

    // delete the callback
    const size_t removed = subscribe_fn_map_.erase(subscription_id);
    assert(removed == 1);

    if (subscribers_.empty()) {
        MessageBuilder presence_unsubscribe(Topic::PRESENCE, Action::UNSUBSCRIBE);
        send_(presence_unsubscribe);
    }
}

void Presence::unsubscribe()
{
    assert(subscribe_fn_map_.size() == subscribers_.size());

    subscribe_fn_map_.clear();
    subscribers_.clear();

    MessageBuilder presence_unsubscribe(Topic::PRESENCE, Action::UNSUBSCRIBE);
    send_(presence_unsubscribe);
}

void Presence::get_all(const QueryFn& f)
{
    querents_.push_back(f);

    if (querents_.size() == 1) {
        MessageBuilder uqq(Topic::PRESENCE, Action::QUERY);
        uqq.add_argument("Q");
        send_(uqq);
    }
}

void Presence::notify_(const Message& message)
{
    assert(message.topic() == Topic::PRESENCE);

    if (message.action() == Action::SUBSCRIBE && message.is_ack())
        return;
    if (message.action() == Action::UNSUBSCRIBE && message.is_ack())
        return;

    if (message.action() == Action::QUERY) {
        UserList users;
        for (std::size_t i = 0; i < message.num_arguments(); ++i)
            users.emplace_back(message[i]);

        for (const QueryFn& f : querents_)
            f(users);

        querents_.clear();
        return;
    }

    if (message.action() != Action::PRESENCE_JOIN && message.action() != Action::PRESENCE_LEAVE) {
        assert(0);
        return;
    }

    bool is_login = message.action() == Action::PRESENCE_JOIN;

    // copy the list because subscribers may unsubscribe in the range-based for
    // loop  below
    SubscriberList subscribers(subscribers_);

    for (const SubscriptionId id : subscribers) {
        SubscribeFn &callback = subscribe_fn_map_[id];
        callback(message[0], is_login);
    }
}
}
