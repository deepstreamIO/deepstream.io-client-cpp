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

namespace deepstream {

    using json = nlohmann::json;

    struct Deepstream {

        Deepstream(std::string& uri)
            : wsh_()
            , error_handler_()
            , client_(uri, wsh_, error_handler_)
            , event(client_)
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
            typedef deepstream::Event::SubscriptionId SubscriptionId;

            Event(Client &client)
                : client_(client)
            {}

            void emit(std::string name, json data) {
                std::string data_str = "O" + data.dump();
                client_.event.emit(Buffer(name.cbegin(), name.cend()), Buffer(data_str.cbegin(), data_str.cend()));
            }

            SubscriptionId subscribe(std::string name, SubscribeFn callback) {
                deepstream::Event::SubscribeFn core_callback([=](const Buffer &data){
                        callback(json::parse(data));
                        });
                return client_.event.subscribe(Buffer(name.cbegin(), name.cend()), callback);
            }

            void unsubscribe(std::string name, SubscriptionId id) {
                client_.event.unsubscribe(Buffer(name.cbegin(), name.cend()), id);
            }

            Client &client_;
        };
        struct Presence {
            Presence() = default;
        };

        Buffer to_buffer(json &data);

        Buffer to_buffer_prefixed(json &data);

        json to_json(Buffer &data);

        json prefixed_to_json(Buffer &data);

    public:
        Event event;
        Presence presence;
    };

    Buffer Deepstream::to_buffer(json &data) {

        return Buffer();
    }

    Buffer Deepstream::to_buffer_prefixed(json &data) {
        return Buffer();
    }

    void to_json(json &j, const Buffer &buff) {
        switch (buff[0]) {
            case 'S':
                {
                    j = json(std::string(buff.data()));
                } break;
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
        return json({});
    }

    void from_json(const json &j, Buffer &buff) {
        return json({});
    }

    json Deepstream::prefixed_to_json(Buffer &data) {
        return json({});
    }

    //struct Deepstream::Presence {
    //};
}

#endif
