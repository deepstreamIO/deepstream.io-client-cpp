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

#include <deepstream/lib/poco-ws.hpp>

#include <algorithm> // std::max

#ifndef NDEBUG
#include <iostream>
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

namespace deepstream {

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

    using namespace Poco::Net;

    PocoWSHandler::PocoWSHandler()
        : WSHandler()
        , uri_()
        , session_(nullptr)
        , websocket_(nullptr)
        , state_(WSState::CLOSED)
        , on_open_(nullptr)
        , on_close_(nullptr)
        , on_message_(nullptr)
        , on_error_(nullptr)
    {
    }

    PocoWSHandler::~PocoWSHandler()
    {
    }

    void PocoWSHandler::process_messages()
    {
        int bytes_received = 0;
        int flags = 0;

        const std::size_t min_buffer_size = 256;
        const std::size_t bytes_available = websocket_->available();
        Buffer buffer(std::max(bytes_available, min_buffer_size), 0);

        try {
            bytes_received = websocket_->receiveFrame(buffer.data(), buffer.size(), flags);
        } catch (Poco::TimeoutException &e) {
            // Received nothing before timeout occurred, but why did we try to
            // read from a socket without any data?
            DEBUG_MSG("Socket read timeout");
            return;
        } catch (WebSocketException &e) {
            state(WSState::CLOSED);
            if (on_error_) {
                const WSHandler::HandlerWithMsgFn &on_error_callback = *on_error_;
                on_error_callback(e.displayText());
            }
            return;
        }

        if (bytes_received == 0) {
            state(WSState::ERROR);
            if (on_error_) {
                WSHandler::HandlerWithMsgFn on_error_callback = *on_error_;
                on_error_callback("Tried to read from closed socket");
            }
            return;
        }

        const int eof_flags = WebSocket::FrameFlags::FRAME_FLAG_FIN
                            | WebSocket::FrameOpcodes::FRAME_OP_CLOSE;

        if (flags == eof_flags) {
            state(WSState::CLOSED);
            if (on_close_) {
                const auto on_close_callback = *on_close_;
                on_close_callback();
            }
            return;
        }

        buffer.resize(bytes_received);

        assert(on_message_);
        const WSHandler::HandlerWithBufFn &on_message_callback = *on_message_;
        on_message_callback(buffer);
    }

    std::string PocoWSHandler::URI() const
    {
        return uri_.toString();
    }

    void PocoWSHandler::URI(std::string uri)
    {
        if (state_ == WSState::OPEN) {
            close();
        }
        uri_ = uri;
    }

    bool PocoWSHandler::send(const Buffer& buffer)
    {
        if (state_ != WSState::OPEN)
        {
            (*on_error_)("Unable to send message on closed socket");
            return false;
        }
        int bytes_sent = 0;
        try {
            bytes_sent = websocket_->sendFrame(buffer.data(), buffer.size());
        } catch (Poco::IOException &e) {
            state(WSState::ERROR);
            (*on_error_)("IO Exception: " + e.displayText());
            return false;
        }
        if (bytes_sent != static_cast<int>(buffer.size())) {
            (*on_error_)("Failed to send full buffer. Sent " +
                    std::to_string(bytes_sent) + " of " + std::to_string(buffer.size()));
        }
        return true;
    }

    WSState PocoWSHandler::state() const
    {
        return state_;
    }

    void PocoWSHandler::state(const WSState state)
    {
        DEBUG_MSG("Websocket State Changed: " << static_cast<int>(state));
        state_ = state;
    }

    void PocoWSHandler::open()
    {
        assert(on_message_);
        if (uri_.empty()) {
            throw std::runtime_error("Cannot open websocket when no URI is set");
        }
        if (uri_.getScheme() == "wss") {
            session_ = std::unique_ptr<HTTPClientSession>(
                    new HTTPSClientSession(uri_.getHost(), uri_.getPort()));
        } else {
            session_ = std::unique_ptr<HTTPClientSession>(
                    new HTTPClientSession(uri_.getHost(), uri_.getPort()));
        }
        try {
            HTTPRequest request(HTTPRequest::HTTP_GET, uri_.getPath(), HTTPRequest::HTTP_1_1);
            HTTPResponse response;
            websocket_ = std::unique_ptr<WebSocket>(new WebSocket(*session_, request, response));
        } catch (NoMessageException &e) {
            if (on_error_) {
                WSHandler::HandlerWithMsgFn on_error_callback = *on_error_;
                on_error_callback(e.displayText());
            } else {
                throw e;
            }
        }
        state(WSState::OPEN);
        if (on_open_) {
            const auto on_open_callback = *on_open_;
            on_open_callback();
        }
    }

    void PocoWSHandler::close()
    {
        if (state_ == WSState::OPEN) {
            websocket_->close();
            state(WSState::CLOSED);
            if (on_close_) {
                const auto on_close_callback = *on_close_;
                on_close_callback();
            }
        }
    }

    void PocoWSHandler::reconnect()
    {
        close();
        open();
    }

    void PocoWSHandler::shutdown()
    {
        if (state_ == WSState::OPEN) {
            websocket_->shutdown();
        }
    }

    void PocoWSHandler::on_close(const HandlerFn& on_close)
    {
        on_close_ = std::unique_ptr<HandlerFn>(new HandlerFn(on_close));
    }

    void PocoWSHandler::on_error(const HandlerWithMsgFn& on_error)
    {
        on_error_ = std::unique_ptr<HandlerWithMsgFn>(new HandlerWithMsgFn(on_error));
    }

    void PocoWSHandler::on_message(const HandlerWithBufFn& on_message)
    {
        on_message_ = std::unique_ptr<HandlerWithBufFn>(new HandlerWithBufFn(on_message));
    }

    void PocoWSHandler::on_open(const HandlerFn& on_open)
    {
        on_open_ = std::unique_ptr<HandlerFn>(new HandlerFn(on_open));
    }

    static SSLManager* sslManager = SSLManager::instance();

}
