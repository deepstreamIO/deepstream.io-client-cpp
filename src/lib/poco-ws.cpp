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

#include <deepstream/core/ws.hpp>

namespace deepstream {

class PocoWS : public deepstream::websocket {
public:
    PocoWS(const std::string& uri)
	: uri_(uri) {};

    ~PocoWS() {};

    std::string URI() const {
	return uri_;
    }

    bool send(const std::string&) {
	return false;
    }

    bool open() {
	return false;
    };

    void close() {}

    void onClose(deepstream::websocket::HandlerFn&){};
    void onError(const deepstream::websocket::HandlerFn){};
    void onMessage(const deepstream::websocket::HandlerFn){};
    void onOpen(const deepstream::websocket::HandlerFn){};

private:
    PocoWS(){};
    std::string uri_;
};
}
