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
#ifndef DEEPSTREAM_HPP
#define DEEPSTREAM_HPP

#include <deepstream/core/buffer.hpp>
#include <deepstream/core/client.hpp>
#include <deepstream/core/error_handler.hpp>
#include <deepstream/core/event.hpp>
#include <deepstream/core/presence.hpp>
#include <deepstream/core/version.hpp>
#include <deepstream/lib/poco-ws.hpp>
#include <deepstream/lib/basic-error-handler.hpp>
#include <deepstream/lib/json.hpp>
#include <deepstream/lib/type-serializer.hpp>

#include <string>
#include <exception>

namespace deepstream {

    using json = nlohmann::json;

    /**
     * The deepstream.io C++ client
     *
     * This class handles all public deepstream functionality through a json
     * interface.
     *
     * Single-threaded, blocking WebSockets are implemented using the POCO
     * library. The user must regularly poll Deepstream::process_messages() to
     * check for new messages, and run the necessary handlers.
     *
     * @see <a href="https://pocoproject.org/docs/Poco.Net.WebSocket.html">
     *          POCO WebSocket Documentation
     *      </a>
     */
    class Deepstream {

    public:
        /**
         * A function that will be invoked with user data upon successful
         * login.
         *
         * @see <a href="https://deepstream.io/tutorials/core/security-overview/#authentication">
         *          deepstream.io authentication overview
         *      </a>
         *
         * @param client_data Client data received from the server.
         */
        typedef std::function<void(const json &&client_data)> LoginCallback;

        Deepstream() = delete;

        Deepstream(const Deepstream &) = delete;

        Deepstream &operator=(const Deepstream &) = delete;

        /**
         * Instantiate the deepstream client and open a connection.
         *
         * Note: The user should make a call to login() soon after
         * instantiating the client, or the server may terminate the
         * connection (configurable with server config option
         * `unauthenticatedClientTimeout`).
         *
         * @param[in] uri The URI of the deepstream server to connect to,
         *                  including the path. e.g. "0.0.0.0:6020/deepstream".
         */
        Deepstream(const std::string &uri)
            : wsh_()
            , error_handler_()
            , client_(uri, wsh_, error_handler_)
            , type_serializer_(error_handler_)
            , event(client_, error_handler_, type_serializer_)
            , presence(client_, error_handler_)
        {
        }

        /**
         * This function reads all incoming messages from the websocket and
         * executes the appropriate callbacks; it blocks until there are no
         * remaining messages to be read.
         */
        void process_messages()
        {
            wsh_.process_messages();
        }

        /**
         * Try to login anonymously.
         *
         * @see login(const json::object_t &auth, const LoginCallback &callback)
         */
        void login()
        {
            const json::object_t auth({});
            login(auth);
        }

        /**
         * Try to login with the given auth data.
         *
         * @see login(const json::object_t &auth, const LoginCallback &callback)
         *
         * @param[in] auth User authentication data
         */
        void login(const json::object_t &auth)
        {
            const LoginCallback callback;
            login(auth, callback);
        }

        /**
         * Try to login anonymously, with a callback giving auth data on
         * success.
         *
         * @see login(const json::object_t &auth, const LoginCallback &callback)
         *
         * @param[in] callback A function that will be invoked with user data
         *                      upon successful login.
         */
        void login(const LoginCallback &callback)
        {
            login(json({}), callback);
        }

        /**
         * Try to login with the given auth data.
         *
         * Given a client in `AWAIT_CONNECTION` state, this function attempts to
         * log in with the given authentication data.
         *
         * The format of the user authentication data depends on the
         * authentication method used by the server.
         *
         * Login occurs asynchronously, so the client will operate in 'offline
         * mode' until the `ConnectionState` is `OPEN`.
         *
         * @see <a href="https://deepstream.io/tutorials/core/security-overview/#authentication">
         *          deepstream.io authentication overview
         *      </a>
         *
         * @param[in] auth User authentication data.
         * @param[in] callback A function that invoked with user data upon
         *                      successful login.
         */
        void login(const json::object_t &auth, const LoginCallback &callback)
        {
            const std::string auth_str(json(auth).dump());
            const Buffer auth_buff(auth_str);

            const Client::LoginCallback core_callback([callback, this](const Buffer &&login_data) {
                const json data = type_serializer_.prefixed_to_json(login_data);
                callback(std::move(data));
            });

            client_.login(auth_buff, core_callback);
        }

        void close();

        /**
         * Retrieve the current connection state of the client.
         */
        ConnectionState get_connection_state() const
        {
            return client_.get_connection_state();
        }

        /**
         * This class presents an API for deepstream.io's event functionality.
         *
         * Member functions may only be invoked via the singleton member
         * Deepstream::event.
         *
         * @see <a href="https://deepstream.io/docs/client-js/pubsub-client-event/">
         *          The javascript client event documentation
         *      </a>
         */
        struct EventWrapper {

            typedef std::function<void(const json &data)> SubscribeFn;
            typedef std::function<bool(const std::string &event_name, bool is_subscribed)> ListenFn;

            EventWrapper() = delete;

            EventWrapper(const EventWrapper &) = delete;

            EventWrapper &operator=(const EventWrapper &) = delete;

            EventWrapper(Client &client, ErrorHandler &error_handler, TypeSerializer &type_serializer)
                : client_(client)
                , error_handler_(error_handler)
                , type_serializer_(type_serializer)
            {
            }

            /**
             * Emit/(Publish) an event to all local and remote subscribers.
             *
             * @param[in] name The name of the event to emit.
             * @param[in] data The payload to be sent.
             */
            void emit(const std::string &name, json data)
            {
                const Buffer &data_buff = type_serializer_.to_prefixed_buffer(data);
                const Buffer name_buff(name);
                client_.event.emit(name_buff, data_buff);
            }

            /**
             * Subscribe to an event.
             * A function can be subscribed to many events (or the same event,
             * multiple times) by repeated calls to this function.
             * Likewise, many functions may be subscribed to a single event
             * name.
             *
             * If you have multiple subscribers to an event name and wish to
             * unsubscribe them individually, hold a reference to the returned
             * SubscriptionId.
             * @see unsubscribe(const std::string &, SubscriptionId)
             *
             * @param[in] name The name to subscribe to.
             * @param[in] callback A function that will be invoked with a json
             *                      payload whenever the an there is a local
             *                      call to `emit` or an event is received
             *                      from the server.
             *
             * @return An identifier that represents the subscription.
             */
            SubscriptionId subscribe(const std::string &name, SubscribeFn callback)
            {
                Buffer name_buff(name);
                Event::SubscribeFn core_callback([callback, this](const Buffer &prefixed_buff) {
                    const json &data = type_serializer_.prefixed_to_json(prefixed_buff);
                    callback(data);
                });
                SubscriptionId subscription_id = client_.event.subscribe(name_buff, core_callback);
                return subscription_id;
            }

            /**
             * Unsubscribe from an event with subscription id.
             *
             * @see subscribe(const std::string &, SubscribeFn)
             *
             * @param[in] name The event name.
             * @param[in] subscription_id The identifier from a call to event.subscribe.
             */
            void unsubscribe(const std::string &name, SubscriptionId subscription_id)
            {
                Buffer name_buff(name);
                client_.event.unsubscribe(name_buff, subscription_id);
            }

            /**
             * Unsubscribe from an event.
             * If more than one function is subscribed to the given event, they
             * will all be unsubscribed.
             *
             * @see subscribe(const std::string &, SubscribeFn)
             *
             * @param[in] name The event name.
             */
            void unsubscribe(const std::string &name)
            {
                Buffer name_buff(name);
                client_.event.unsubscribe(name_buff);
            }

            /**
             * Register the client as a listener for event subscriptions made
             * by other clients.
             * Any subscriptions to event names matching the given pattern will
             * trigger a listen callback.
             *
             * @param[in] pattern The pattern to match
             * @param[in] callback A function that will be called whenever an
             *                      event has been initially subscribed to or
             *                      is no longer subscribed.
             */
            void listen(const std::string &pattern, const ListenFn callback)
            {
                Event::ListenFn core_callback([callback](const Event::Name &match, bool is_subscribed) -> bool {
                    const std::string pattern_str(match.data(), match.size());
                    return callback(pattern_str, is_subscribed);
                });
                Event::Name pattern_buff(pattern);
                client_.event.listen(pattern_buff, core_callback);
            }

            /**
             * Unregister a listener
             * The pattern must equal that which was used in the call to
             * listen(const std::string, const ListenFn)
             *
             * @param pattern The pattern to unregister.
             */
            void unlisten(const std::string &pattern)
            {
                Buffer pattern_buff(pattern);
                client_.event.unlisten(pattern_buff);
            }

        private:
            Client &client_;
            ErrorHandler &error_handler_;
            TypeSerializer &type_serializer_;
        };

        /**
         * This class presents an API for deepstream.io's presence functionality.
         *
         * Member functions may only be invoked via the singleton member
         * Deepstream::presence.
         *
         * @see <a href="https://deepstream.io/docs/client-js/presence/">
         *          The javascript client event documentation
         *      </a>
         */
        struct PresenceWrapper {

            /**
             * @param name The username of a client that has logged in or out.
             * @param online `true` if client logged in, `false` if client
             *                  logged out
             */
            typedef std::function<void(const std::string &name, bool online)> SubscribeFn;

            typedef std::vector<std::string> UserList;
            typedef std::function<void(const UserList&)> QueryFn;

            PresenceWrapper() = delete;

            PresenceWrapper(const PresenceWrapper &) = delete;

            PresenceWrapper &operator=(const PresenceWrapper &) = delete;

            PresenceWrapper(Client &client, ErrorHandler &error_handler)
                : client_(client)
                , error_handler_(error_handler)
            {
            }

            /**
             * Subscribe to presence events.
             *
             * Whenever a client successfully authenticates with a username or
             * logs out, `callback` will be invoked with the username of the
             * client and a `bool` 'online'. 'online' indicates whether the
             * event is a login(true) or logout(false).
             *
             * If you have multiple presence subscribers and wish to
             * unsubscribe them individually, hold a reference to the returned
             * SubscriptionId.
             * @see unsubscribe(SubscriptionId)
             *
             * @param callback A function that accepts a username and a bool (called
             *                  repeatedly).
             * @return An identifier that represents the subscription.
             */
            SubscriptionId subscribe(SubscribeFn callback)
            {
                Presence::SubscribeFn core_callback([callback, this](const Buffer &name, bool online) {
                    callback(std::string(name.data(), name.size()), online);
                });
                return client_.presence.subscribe(core_callback);
            }

            /**
             * Unsubscribe from a specific presence subscription.
             *
             * @see subscribe(SubscribeFn).
             *
             * @param id The subscription id.
             */
            void unsubscribe(SubscriptionId id)
            {
                client_.presence.unsubscribe(id);
            }

            /**
             * Unsubscribe from all presence subscriptions.
             */
            void unsubscribe()
            {
                client_.presence.unsubscribe();
            }

            /**
             * Query for all currently connected clients.
             *
             * @param callback A function that accepts a list of usernames
             *                  (called once).
             */
            void get_all(QueryFn callback)
            {
                Presence::QueryFn core_callback([callback, this](const std::vector<Buffer> users) {
                    std::vector<std::string> users_str(users.size());
                    for (std::size_t i = 0; i < users.size(); ++i) {
                        const Buffer &user = users[i];
                        users_str[i] = std::string(user.data(), user.size());
                    }
                    callback(users_str);
                });
                client_.presence.get_all(core_callback);
            }

        private:
            Client &client_;
            ErrorHandler &error_handler_;
        };

    private:
        PocoWSHandler wsh_;
        BasicErrorHandler error_handler_;
        Client client_;
        TypeSerializer type_serializer_;

    public:
        EventWrapper event;
        PresenceWrapper presence;
    };
    }

#endif // DEEPSTREAM_HPP
