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

#pragma once

namespace deepstream {
class websocket {
public:
    websocket(const std::string& URI);
    virtual ~websocket() {};
    virtual bool send(const std::string& msg) = 0;
    virtual std::string URI() const = 0;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual void onOpen(const std::string& msg) const = 0;
    virtual void onClose(const std::string& msg) const = 0;
    virtual void onError(const std::string& msg) const = 0;
    virtual void onMessage(const std::string& msg) const = 0;
private:
    websocket() = delete;
    websocket(websocket const&) = delete;
    websocket& operator=(websocket const&) = delete;
};
}
