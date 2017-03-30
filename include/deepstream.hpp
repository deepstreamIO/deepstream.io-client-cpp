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

#include <string>

namespace deepstream {

    struct Deepstream {

        Deepstream(std::string& uri)
            : wsh_()
            , error_handler_()
            , client_(uri, wsh_, error_handler_)
            , event(client_.event)
            , presence(client_.presence)
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
            return login("{}", [](const Buffer &){});
        }

        void login(const Client::LoginCallback &callback)
        {
            return login("{}", callback);
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
        void login(const std::string &auth, const Client::LoginCallback &callback)
        {
            client_.login(auth, callback);
        }

        void close();

        ConnectionState get_connection_state(){
            return client_.get_connection_state();
        }


    private:

        PocoWSHandler wsh_;
        BasicErrorHandler error_handler_;
        Client client_;

    public:
        Event &event;
        Presence &presence;
    };

}

#endif
