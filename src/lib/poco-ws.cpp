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

#include <Poco/Exception.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/URI.h>

#include <deepstream/poco/ws.hpp>

class SSLManager {
public:
    SSLManager()
        : pAcceptCertHandler_(nullptr)
        , pContext_(nullptr)
    {
        Poco::Net::HTTPStreamFactory::registerFactory();
        Poco::Net::HTTPSStreamFactory::registerFactory();

        pAcceptCertHandler_ = new Poco::Net::AcceptCertificateHandler(true);
        pContext_ = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE,
            "",
            "",
            "",
            // TODO(frobware) VERIFY_NONE is pointless. Should be VERIFY_STRICT.
            Poco::Net::Context::VERIFY_NONE,
            9,
            true,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        Poco::Net::SSLManager::instance().initializeClient(NULL, pAcceptCertHandler_, pContext_);
    }

    ~SSLManager() = default;

    static SSLManager* instance()
    {
        static SSLManager* m = nullptr;
        if (m == nullptr) {
            m = new SSLManager();
        }
        return m;
    }

private:
    Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pAcceptCertHandler_;
    Poco::Net::Context::Ptr pContext_;
};

struct PocoWS : public deepstream::WS {
public:
    static Poco::Net::HTTPClientSession* newSession(const std::string& uri_string)
    {
        Poco::URI uri(uri_string);

        if (uri.getScheme() == "wss") {
            return new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
        } else {
            return new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
        }
    }

    explicit PocoWS(const std::string& uri)
        : uri_(uri)
        , session_(newSession(uri))
        , request_(Poco::Net::HTTPRequest::HTTP_GET, uri_.getPath(), Poco::Net::HTTPRequest::HTTP_1_1)
        , websocket_(*session_, request_, response_)
    {
    }

    virtual ~PocoWS()
    {
        delete session_;
    }

    std::string URI() const
    {
        return uri_.toString();
    }

    int send(const void *buffer, int length) const
    {
        return 0;
    }

    int recv(void *buffer, int length, int& flags) const
    {
        return 0;
    }

    bool open()
    {
        return false;
    };

    void close() {}
    void shutdown() {}

    void onClose(const HandlerFn&) const override {}
    void onError(const HandlerWithMsgFn&) const override {}
    void onMessage(const HandlerWithMsgFn&) const override {}
    void onOpen(const HandlerFn&) const override {}

    int available() const override {
	return 0;
    }

private:
    Poco::URI uri_;
    Poco::Net::HTTPClientSession* session_;
    Poco::Net::HTTPRequest request_;
    Poco::Net::HTTPResponse response_;
    Poco::Net::WebSocket websocket_;
};

deepstream::WS* PocoWSFactory::connect(const std::string& uri)
{
    return new PocoWS(uri);
}

PocoWSFactory* PocoWSFactory::instance()
{
    static PocoWSFactory* f = nullptr;

    if (f == nullptr) {
        f = new PocoWSFactory();
    }

    return f;
}

static PocoWSFactory* instance = PocoWSFactory::instance();
static SSLManager* sslManager = SSLManager::instance();
