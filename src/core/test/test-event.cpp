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
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <deepstream/core/buffer.hpp>
#include <deepstream/core/event.hpp>
#include "../message.hpp"
#include "../message_builder.hpp"
#include "../scope_guard.hpp"

namespace deepstream {

BOOST_AUTO_TEST_CASE(simple)
{
    typedef Event::Name Name;

    int state = -1;
    const Event::Name name("name");
    const Event::Name pattern("pattern");

    auto send = [&state, name, pattern](const Message& message) {
        if (state == -1)
            BOOST_FAIL("Internal state is faulty");

        DEEPSTREAM_ON_EXIT([&state]() { state = -1; });

        BOOST_CHECK_EQUAL(message.topic(), Topic::EVENT);

        if (state == 0) {
            BOOST_CHECK_EQUAL(message.action(), Action::SUBSCRIBE);
            BOOST_CHECK(!message.is_ack());

            BOOST_REQUIRE_EQUAL(message.num_arguments(), 1);
            const Event::Name& my_name = message[0];
            BOOST_REQUIRE_EQUAL(name.size(), my_name.size());
            BOOST_CHECK(std::equal(name.cbegin(), name.cend(), my_name.cbegin()));

            return true;
        }

        if (state == 1) {
            BOOST_CHECK_EQUAL(message.action(), Action::LISTEN);
            BOOST_CHECK(!message.is_ack());

            BOOST_REQUIRE_EQUAL(message.num_arguments(), 1);
            const Event::Name& my_pattern = message[0];
            BOOST_REQUIRE_EQUAL(pattern.size(), my_pattern.size());
            BOOST_CHECK(
                std::equal(pattern.cbegin(), pattern.cend(), my_pattern.cbegin()));

            return true;
        }

        if (state == 2) {
            BOOST_CHECK_EQUAL(message.action(), Action::UNSUBSCRIBE);
            BOOST_CHECK(!message.is_ack());

            BOOST_REQUIRE_EQUAL(message.num_arguments(), 1);
            const Event::Name& my_name = message[0];
            BOOST_REQUIRE_EQUAL(name.size(), my_name.size());
            BOOST_CHECK(std::equal(name.cbegin(), name.cend(), my_name.cbegin()));

            return true;
        }

        if (state == 3) {
            BOOST_CHECK_EQUAL(message.action(), Action::UNLISTEN);
            BOOST_CHECK(!message.is_ack());

            BOOST_REQUIRE_EQUAL(message.num_arguments(), 1);
            const Event::Name& my_pattern = message[0];
            BOOST_REQUIRE_EQUAL(pattern.size(), my_pattern.size());
            BOOST_CHECK(
                std::equal(pattern.cbegin(), pattern.cend(), my_pattern.begin()));

            return true;
        }

        assert(0);
        return false;
    };

    SubscriptionId subscription_counter;
    Event event(send, subscription_counter);

    Event::SubscribeFn f = [](const Buffer&) {};

    state = 0;
    const SubscriptionId s1 = event.subscribe(name, f);
    {
        BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);
        auto it = event.subscriber_map_.find(name);
        BOOST_REQUIRE(it != event.subscriber_map_.end());
        BOOST_CHECK_EQUAL(it->second.size(), 1);
        BOOST_CHECK_EQUAL(it->second.front(), s1);
    }

    const SubscriptionId s2 = event.subscribe(name, f);
    {
        BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);
        auto it = event.subscriber_map_.find(name);
        BOOST_REQUIRE(it != event.subscriber_map_.end());
        BOOST_CHECK_EQUAL(it->second.size(), 2);
    }

    event.unsubscribe(name, s1);
    {
        BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);
        auto it = event.subscriber_map_.find(name);
        BOOST_REQUIRE(it != event.subscriber_map_.end());
        BOOST_CHECK_EQUAL(it->second.size(), 1);
    }

    BOOST_CHECK(event.listener_map_.empty());

    Event::ListenFn g = [](const Name&, bool) { return true; };
    Event::ListenFnPtr q1(new Event::ListenFn(g));
    Event::ListenFnPtr q2(new Event::ListenFn(g));

    state = 1;
    event.listen(pattern, q1);
    {
        BOOST_CHECK_EQUAL(event.listener_map_.size(), 1);
        auto it = event.listener_map_.find(pattern);
        BOOST_REQUIRE(it != event.listener_map_.end());
        BOOST_REQUIRE_EQUAL(pattern.size(), it->first.size());
        BOOST_REQUIRE(
            std::equal(pattern.cbegin(), pattern.cend(), it->first.cbegin()));
        BOOST_REQUIRE_EQUAL(it->second, q1);
    }

    event.listen(pattern, q2);
    {
        BOOST_CHECK_EQUAL(event.listener_map_.size(), 1);
        auto it = event.listener_map_.find(pattern);
        BOOST_REQUIRE(it != event.listener_map_.end());
        BOOST_REQUIRE_EQUAL(pattern.size(), it->first.size());
        BOOST_REQUIRE(
            std::equal(pattern.cbegin(), pattern.cend(), it->first.cbegin()));
        BOOST_REQUIRE_EQUAL(it->second, q1);
    }

    BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);
    event.unsubscribe(name, s1); // we removed s1 above already
    BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);

    state = 2;
    event.unsubscribe(name, s2);
    BOOST_CHECK(event.subscriber_map_.empty());
    BOOST_CHECK_EQUAL(event.listener_map_.size(), 1);

    state = 3;
    event.unlisten(pattern);
    BOOST_CHECK(event.subscriber_map_.empty());
    BOOST_CHECK(event.listener_map_.empty());
}

BOOST_AUTO_TEST_CASE(subscriber_notification)
{
    const Event::Name name("name");
    const Buffer data("data");

    bool is_subscribed = false;
    auto send = [name, &is_subscribed](const Message& message) -> bool {
        const Event::Name& my_name = message[0];

        BOOST_CHECK_EQUAL(message.topic(), Topic::EVENT);
        BOOST_REQUIRE_EQUAL(name.size(), my_name.size());
        BOOST_CHECK(std::equal(name.cbegin(), name.cend(), my_name.cbegin()));

        if (message.action() == Action::SUBSCRIBE)
            is_subscribed = true;
        else if (message.action() == Action::UNSUBSCRIBE)
            is_subscribed = false;
        else
            BOOST_FAIL("This branch should not be taken");

        return true;
    };

    SubscriptionId subscription_counter;
    Event event(send, subscription_counter);

    unsigned num_calls = 0;
    Event::SubscribeFn f = [data, &num_calls](const Buffer& my_data) {
        BOOST_CHECK(std::equal(data.cbegin(), data.cend(), my_data.cbegin()));
        ++num_calls;
    };

    SubscriptionId sub_id = event.subscribe(name, f);
    BOOST_CHECK(is_subscribed);
    BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);

    MessageBuilder message(Topic::EVENT, Action::EVENT);
    message.add_argument(name);
    message.add_argument(data);

    event.notify_(message);
    BOOST_CHECK_EQUAL(num_calls, 1);

    Event::SubscribeFn g = [name, data, &num_calls, &event, sub_id](const Buffer& my_data) {
        BOOST_CHECK(std::equal(data.cbegin(), data.cend(), my_data.cbegin()));

        num_calls += 10;
        event.unsubscribe(name, sub_id);
    };

    SubscriptionId p_g = event.subscribe(name, g);
    event.notify_(message);
    BOOST_CHECK_EQUAL(num_calls, 12);

    BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);

    Event::SubscriberMap::const_iterator ci = event.subscriber_map_.cbegin();
    const Event::SubscriberList& subscribers = ci->second;
    BOOST_CHECK_EQUAL(subscribers.size(), 1);
    BOOST_CHECK_EQUAL(subscribers.front(), p_g);

    event.unsubscribe(name);
    BOOST_CHECK(event.subscriber_map_.empty());
    BOOST_CHECK(!is_subscribed);
}

BOOST_AUTO_TEST_CASE(emit)
{
    const Event::Name name("name");
    const Buffer data("data");

    bool is_subscribed = false;
    unsigned num_emit = 0;
    auto send = [name, data, &is_subscribed, &num_emit](const Message& message) {
        BOOST_CHECK_EQUAL(message.topic(), Topic::EVENT);

        const Event::Name& my_name = message[0];
        BOOST_REQUIRE_EQUAL(name.size(), my_name.size());
        BOOST_CHECK(std::equal(name.cbegin(), name.cend(), my_name.cbegin()));

        if (message.action() == Action::SUBSCRIBE)
            is_subscribed = true;
        else if (message.action() == Action::UNSUBSCRIBE)
            is_subscribed = false;
        else if (message.action() == Action::EVENT) {
            BOOST_REQUIRE_EQUAL(message.num_arguments(), 2);

            const Buffer& my_data = message[1];
            BOOST_REQUIRE_EQUAL(data.size(), my_data.size());
            BOOST_CHECK(std::equal(data.cbegin(), data.cend(), my_data.cbegin()));

            ++num_emit;
        } else
            BOOST_FAIL("This branch should not be taken");

        return true;
    };

    SubscriptionId subscription_counter;
    Event event(send, subscription_counter);

    unsigned num_calls = 0;
    Event::SubscribeFn f = [data, &num_calls](const Buffer& my_data) {
        BOOST_CHECK(std::equal(data.cbegin(), data.cend(), my_data.cbegin()));
        ++num_calls;
    };

    SubscriptionId sub = event.subscribe(name, f);
    BOOST_CHECK(is_subscribed);
    BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);

    event.emit(name, data);
    BOOST_CHECK_EQUAL(num_emit, 1);
    BOOST_CHECK_EQUAL(num_calls, 1);

    Event::SubscribeFn g = [name, data, &num_calls, &event, sub](const Buffer& my_data) {
        BOOST_CHECK(std::equal(data.cbegin(), data.cend(), my_data.cbegin()));

        num_calls += 10;
        event.unsubscribe(name, sub);
    };
    SubscriptionId p_g = event.subscribe(name, g);

    event.emit(name, data);
    BOOST_CHECK_EQUAL(num_emit, 2);
    BOOST_CHECK_EQUAL(num_calls, 12);

    BOOST_CHECK_EQUAL(event.subscriber_map_.size(), 1);
    Event::SubscriberMap::const_iterator ci = event.subscriber_map_.cbegin();
    const Event::SubscriberList& subscribers = ci->second;
    BOOST_CHECK_EQUAL(subscribers.size(), 1);
    BOOST_CHECK_EQUAL(subscribers.front(), p_g);

    event.unsubscribe(name);
    BOOST_CHECK(event.subscriber_map_.empty());
    BOOST_CHECK(!is_subscribed);
}

BOOST_AUTO_TEST_CASE(listener_notification)
{
    typedef Event::Name Name;

    const Name pattern(".*");
    const Name match("match");

    bool is_listening = false;
    bool sent_accept = false;
    auto send = [pattern, &is_listening, &sent_accept](const Message& message) {
        BOOST_CHECK_EQUAL(message.topic(), Topic::EVENT);
        BOOST_CHECK(!message.is_ack());
        BOOST_REQUIRE_EQUAL(message.num_arguments(),
            (message.action() == Action::LISTEN_ACCEPT) ? 2 : 1);

        const Event::Name& my_pattern = message[0];
        BOOST_REQUIRE_EQUAL(pattern.size(), my_pattern.size());
        BOOST_CHECK(
            std::equal(pattern.cbegin(), pattern.cend(), my_pattern.cbegin()));

        if (message.action() == Action::LISTEN)
            is_listening = true;
        else if (message.action() == Action::UNLISTEN)
            is_listening = false;
        else if (is_listening && message.action() == Action::LISTEN_ACCEPT)
            sent_accept = true;
        else
            BOOST_FAIL("This branch should not be taken");

        return true;
    };

    bool has_subscriber = false;
    Event::ListenFn f = [match, &has_subscriber](const Name& my_match, bool b) {
        BOOST_REQUIRE_EQUAL(match.size(), my_match.size());
        BOOST_CHECK(std::equal(match.cbegin(), match.cend(), my_match.cbegin()));

        has_subscriber = b;

        return true;
    };

    SubscriptionId subscription_counter;
    Event event(send, subscription_counter);
    Event::ListenFnPtr p_f = event.listen(pattern, f);

    BOOST_CHECK(is_listening);
    BOOST_CHECK(p_f);
    BOOST_CHECK_EQUAL(event.listener_map_.size(), 1);
    BOOST_CHECK_EQUAL(event.listener_map_[pattern], p_f);

    // E|SP
    MessageBuilder sp(Topic::EVENT, Action::SUBSCRIPTION_FOR_PATTERN_FOUND);
    sp.add_argument(pattern);
    sp.add_argument(match);

    BOOST_CHECK(!has_subscriber);
    BOOST_CHECK(!sent_accept);
    event.notify_(sp);
    BOOST_CHECK(has_subscriber);
    BOOST_CHECK(sent_accept);

    // E|SR
    MessageBuilder sr(Topic::EVENT, Action::SUBSCRIPTION_FOR_PATTERN_REMOVED);
    sr.add_argument(pattern);
    sr.add_argument(match);

    event.notify_(sr);

    BOOST_CHECK(!has_subscriber);

    // unlisten
    event.unlisten(pattern);

    BOOST_CHECK(!is_listening);
    BOOST_CHECK(event.listener_map_.empty());
}
}
