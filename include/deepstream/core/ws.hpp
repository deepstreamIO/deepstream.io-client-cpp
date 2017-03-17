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

#include <functional>
#include <string>

namespace deepstream {
class WS {
public:
    typedef std::function<void(const std::string&)> HandlerWithMsgFn;
    typedef std::function<void()> HandlerFn;

    WS(){};

    virtual ~WS(){};

    virtual std::string URI() const = 0;

    virtual bool send(const std::string& msg) = 0;

    virtual bool open() = 0;

    virtual void close() = 0;

    virtual void onClose(const HandlerFn&) const = 0;

    virtual void onError(const HandlerWithMsgFn&) const = 0;

    virtual void onMessage(const HandlerWithMsgFn&) const = 0;

    virtual void onOpen(const HandlerFn&) const = 0;

protected:
    WS(WS const&);

    WS& operator=(WS const&);
};

struct WSFactory {
    virtual WS* connect(const std::string& uri) = 0;
    virtual ~WSFactory() {}
};
}
