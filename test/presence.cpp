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

#include <deepstream/buffer.hpp>
#include <deepstream/message.hpp>
#include <deepstream/message_builder.hpp>
#include <deepstream/presence.hpp>

namespace deepstream {

BOOST_AUTO_TEST_CASE(subscription)
{
    typedef Presence::Name Name;

    const Name name("name");

    bool is_subscribed = false;

    Presence::SendFn send = [&is_subscribed](const Message& message) -> bool {
        BOOST_CHECK_EQUAL(message.topic(), Topic::PRESENCE);
        BOOST_CHECK(!message.is_ack());

        if (message.action() == Action::SUBSCRIBE) {
            BOOST_CHECK(!is_subscribed);
            is_subscribed = true;
        } else if (message.action() == Action::UNSUBSCRIBE) {
            BOOST_CHECK(is_subscribed);
            is_subscribed = false;
        } else
            BOOST_FAIL("This branch should not be taken");

        return true;
    };

    unsigned num_calls = 0;
    Presence::SubscribeFn f = [name, &num_calls](const Name& my_name,
        bool is_login) {
        BOOST_REQUIRE_EQUAL(name.size(), my_name.size());
        BOOST_CHECK(std::equal(name.cbegin(), name.cend(), my_name.cbegin()));
        BOOST_CHECK(!is_login);

        ++num_calls;
    };

    Presence presence(send);

    const std::size_t N = 10;
    Presence::SubscriberList subscribers;
    for (std::size_t i = 0; i < N; ++i)
        subscribers.push_back(presence.subscribe(f));

    BOOST_CHECK(is_subscribed);
    BOOST_CHECK_EQUAL(presence.subscribers_.size(), N);

    MessageBuilder pnl(Topic::PRESENCE, Action::PRESENCE_LEAVE);
    pnl.add_argument(name);
    presence.notify_(pnl);

    BOOST_CHECK(is_subscribed);
    BOOST_CHECK_EQUAL(num_calls, 10);

    while (!subscribers.empty()) {
        Presence::SubscribeFnPtr p_f = subscribers.back();

        subscribers.pop_back();
        presence.unsubscribe(p_f);

        BOOST_CHECK_EQUAL(subscribers.size(), presence.subscribers_.size());
    }

    BOOST_CHECK(!is_subscribed);
}

BOOST_AUTO_TEST_CASE(queries)
{
    typedef Presence::Name Name;

    const Name userA("foo");
    const Name userB("bar");

    unsigned num_queries = 0;

    Presence::SendFn send = [&num_queries](const Message& message) -> bool {
        BOOST_CHECK_EQUAL(message.topic(), Topic::PRESENCE);
        BOOST_CHECK(!message.is_ack());
        BOOST_CHECK_EQUAL(message.action(), Action::QUERY);

        ++num_queries;

        return true;
    };

    unsigned num_calls = 0;
    Presence::QueryFn f = [&num_calls, userA,
        userB](const Presence::UserList& users) {
        BOOST_REQUIRE_EQUAL(users.size(), 2);
        BOOST_CHECK(users.front() == userA || users.back() == userA);
        BOOST_CHECK(users.front() == userB || users.back() == userB);

        ++num_calls;
    };

    Presence presence(send);

    presence.get_all(f);
    BOOST_CHECK_EQUAL(num_queries, 1);
    BOOST_CHECK_EQUAL(presence.querents_.size(), 1);

    presence.get_all(f);
    BOOST_CHECK_EQUAL(num_queries, 1);
    BOOST_CHECK_EQUAL(presence.querents_.size(), 2);

    MessageBuilder uq(Topic::PRESENCE, Action::QUERY, true);
    uq.add_argument(userA);
    uq.add_argument(userB);

    presence.notify_(uq);
    BOOST_CHECK(presence.querents_.empty());
    BOOST_CHECK_EQUAL(num_calls, 2);
}
}
