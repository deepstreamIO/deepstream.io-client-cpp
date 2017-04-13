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

#include <string>
#include <exception>

namespace deepstream {

    using json = nlohmann::json;

    struct Deepstream {

        Deepstream(std::string& uri)
            : wsh_()
            , error_handler_()
            , client_(uri, wsh_, error_handler_)
            , event(client_, error_handler_)
        {}

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
            const Client::LoginCallback callback([](const std::unique_ptr<Buffer> &&){});
            login(auth, callback);
        }

        void login(const Client::LoginCallback &callback)
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
         * @param[out] p_user_data On a successful reeturn, store the user data
         * in `p_user_data` if the reference is not `NULL`.
         */
        void login(const json::object_t &auth, const Client::LoginCallback &callback)
        {
            const std::string auth_str(json(auth).dump());
            client_.login(Buffer(auth_str.cbegin(), auth_str.cend()), callback);
        }

        void close();

        ConnectionState get_connection_state(){
            return client_.get_connection_state();
        }


    private:

        PocoWSHandler wsh_;
        BasicErrorHandler error_handler_;
        Client client_;

        struct Event {
            typedef std::function<void(const json&)> SubscribeFn;
            typedef std::map<SubscriptionId, SubscribeFn> SubscriptionMap;
            typedef std::function<bool(const std::string&, bool)> ListenFn;

            Event(Client &client, ErrorHandler &error_handler)
                : client_(client)
                , error_handler_(error_handler)
            {}

            void emit(std::string name, json data) {
                const Buffer &data_buff = to_prefixed_buffer(data);
                const Buffer name_buff(name.cbegin(), name.cend());
                client_.event.emit(name_buff, data_buff);
            }

            SubscriptionId subscribe(std::string name, SubscribeFn callback) {
                Buffer name_buff(name.cbegin(), name.cend());
                deepstream::Event::SubscribeFn core_callback([&this, callback](const Buffer &prefixed_buff){
                        const json &data = prefixed_to_json(prefixed_buff);
                        callback(data);
                        });
                SubscriptionId subscription_id =
                    client_.event.subscribe(name_buff, core_callback);
                subscription_map_[subscription_id] = callback;
                return subscription_id;
            }

            void unsubscribe(std::string name, SubscriptionId subscription_id) {
                Buffer name_buff(name.cbegin(), name.cend());
                client_.event.unsubscribe(name_buff, subscription_id);
            }

            void listen(const std::string &pattern, const ListenFn callback) {
                using Core = deepstream::Event;
                Core::ListenFn core_callback([callback](const Core::Name &match, bool is_subscribed) -> bool{
                        std::string pattern_str(match.data(), match.size());
                        return callback(pattern_str, is_subscribed);
                        });
                Core::Name pattern_buff(pattern.cbegin(), pattern.cend());
                client_.event.listen(pattern_buff, core_callback);
            }

            void unlisten(const std::string &pattern) {
                Buffer pattern_buff(pattern.cbegin(), pattern.cend());
                client_.event.unlisten(pattern_buff);
            }

            SubscriptionMap subscription_map_;
            Client &client_;
            ErrorHandler &error_handler_;
        };
        struct Presence {
            Presence() = default;
        };

        Buffer to_buffer(const json &);

        Buffer to_prefixed_buffer(const json &);

        json to_json(const Buffer &);

        json prefixed_to_json(const Buffer &);

    public:
        Event event;
        Presence presence;
    };

    Buffer Deepstream::to_buffer(const json &data) {

        return Buffer();
    }

    enum class DSType : char {
        STRING = 'S',
        OBJECT = 'O',
        NUMBER = 'N',
        NULL_ = 'L',
        TRUE = 'T',
        FALSE = 'F',
        UNDEFINED = 'U'
    };

    Buffer Deepstream::to_prefixed_buffer(const json &data) {
        DSType prefix;

        if (data.is_string()) {
            prefix = DSType::STRING;
        } else if (data.is_object()) {
            prefix = DSType::OBJECT;
        } else if (data.is_number()) {
            prefix = DSType::NUMBER;
        } else if (data.is_null()) {
            prefix = DSType::NULL_;
        } else if (data.is_boolean()) {
            prefix = data ? DSType::TRUE : DSType::FALSE;
        } else if (false /* undefined isn't in the JSON spec */){
            prefix = DSType::UNDEFINED;
        } else {
            throw new std::runtime_error("Unable to serialize type: " + data.dump());
        }

        if (prefix == DSType::STRING
                || prefix == DSType::OBJECT
                || prefix == DSType::NUMBER) {
            std::string str = data.dump();
            str.insert(0, 1, static_cast<char>(prefix));
            return Buffer(str);
        }

        return Buffer{ static_cast<char>(prefix) };
    }

    json to_json(const Buffer &buff) {
        return json({});
    }

    // void to_json(json &j, const Buffer &buff) { };
    // void from_json(const json &j, Buffer &buff) { return json({}); }

    json Deepstream::prefixed_to_json(const Buffer &buff) {
        switch (buff[0]) {
            case 'S':
                return std::string(buff.data() + 1, buff.size() - 1);
            case 'O':
                {
                } break;
            case 'N':
                {

                } break;
            case 'L':
                {

                } break;
            case 'T':
                {

                } break;
            case 'F':
                {

                } break;
            case 'U':
                {

                } break;
        }
        error_handler_.on_error("Unable to deserialize buffer: "
                + std::string(buff, buff.size());
    }

    //struct Deepstream::Presence {
    //};
}

#endif
