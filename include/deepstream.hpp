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

    struct Deepstream {

        typedef std::function<void(const json &&)> LoginCallback;

        Deepstream() = delete;

        Deepstream(const Deepstream &) = delete;

        Deepstream &operator=(const Deepstream &) = delete;

        Deepstream(std::string &uri)
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
         * executes the appropriate callbacks; it returns when there are no
         * messages left.
         */
        void process_messages()
        {
            wsh_.process_messages();
        }

        void login()
        {
            const json::object_t auth({});
            login(auth);
        }

        void login(const json::object_t &auth)
        {
            const LoginCallback callback;
            login(auth, callback);
        }

        void login(const LoginCallback &callback)
        {
            login(json({}), callback);
        }

        /**
         * Given a client in `AWAIT_CONNECTION` state, this function attempts to
         * log in with the given authentication data.
         *
         * The format of the user authentication data depends on the <a
         * href="https://deepstream.io/tutorials/core/security-overview/">authentication</a>
         * method used by the server.
         *
         * @param[in] auth User authentication data
         * @param[in] callback A LoginCallback that will be invoked with the
         * user data on successful login.
         * in `p_user_data` if the reference is not `NULL`.
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

        ConnectionState get_connection_state()
        {
            return client_.get_connection_state();
        }

    private:
        PocoWSHandler wsh_;
        BasicErrorHandler error_handler_;
        Client client_;

        struct EventWrapper {
            typedef std::function<void(const json &)> SubscribeFn;
            typedef std::function<bool(const std::string &, bool)> ListenFn;

            EventWrapper() = delete;

            EventWrapper(const EventWrapper &) = delete;

            EventWrapper &operator=(const EventWrapper &) = delete;

            EventWrapper(Client &client, ErrorHandler &error_handler, TypeSerializer &type_serializer)
                : client_(client)
                , error_handler_(error_handler)
                , type_serializer_(type_serializer)
            {
            }

            void emit(std::string name, json data)
            {
                const Buffer &data_buff = type_serializer_.to_prefixed_buffer(data);
                const Buffer name_buff(name);
                client_.event.emit(name_buff, data_buff);
            }

            SubscriptionId subscribe(std::string name, SubscribeFn callback)
            {
                Buffer name_buff(name);
                Event::SubscribeFn core_callback([callback, this](const Buffer &prefixed_buff) {
                    const json &data = type_serializer_.prefixed_to_json(prefixed_buff);
                    callback(data);
                });
                SubscriptionId subscription_id = client_.event.subscribe(name_buff, core_callback);
                return subscription_id;
            }

            void unsubscribe(std::string name, SubscriptionId subscription_id)
            {
                Buffer name_buff(name);
                client_.event.unsubscribe(name_buff, subscription_id);
            }

            void listen(const std::string &pattern, const ListenFn callback)
            {
                Event::ListenFn core_callback([callback](const Event::Name &match, bool is_subscribed) -> bool {
                    std::string pattern_str(match.data(), match.size());
                    return callback(pattern_str, is_subscribed);
                });
                Event::Name pattern_buff(pattern);
                client_.event.listen(pattern_buff, core_callback);
            }

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

        struct PresenceWrapper {
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

            SubscriptionId subscribe(SubscribeFn callback)
            {
                Presence::SubscribeFn core_callback([callback, this](const Buffer &name, bool online) {
                    callback(std::string(name.data(), name.size()), online);
                });
                return client_.presence.subscribe(core_callback);
            }

            void unsubscribe(SubscriptionId id)
            {
                client_.presence.unsubscribe(id);
            }

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
        TypeSerializer type_serializer_;

    public:
        EventWrapper event;
        PresenceWrapper presence;
    };
    }

#endif // DEEPSTREAM_HPP
