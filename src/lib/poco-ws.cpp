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

#include <deepstream/core/ws.hpp>

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
    struct PocoWSHandler : public WSHandler {

        explicit PocoWSHandler()
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

        virtual ~PocoWSHandler()
        {
        }

        void process_messages()
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
                state_ = WSState::CLOSED;
                if (on_error_callback) {
                    WSHandler::HandlerWithMsgFn on_error_callback = *on_error_;
                    on_error_callback(e.displayText());
                }
                return;
            }

            if (bytes_received == 0) {
                state_ = WSState::ERROR;
                on_error_callback("Tried to read from closed socket");
                return;
            }

            const int eof_flags = WebSocket::FrameFlags::FRAME_FLAG_FIN
                                | WebSocket::FrameOpcodes::FRAME_OP_CLOSE;

            if (flags == eof_flags) {
                state_ = WSState::CLOSED;
                if (on_close_) {
                    const auto on_close_callback = *on_close_;
                    on_close_callback();
                }
                return;
            }

            assert(on_message_);
            WSHandler::HandlerWithBufFn on_message_callback = *on_message_;
            on_message_callback(buffer);
        }

        std::string URI() const override
        {
            return uri_.toString();
        }

        void URI(std::string uri) override
        {
            uri_ = uri;
        }

        void send(const Buffer& buffer) override
        {
            const int bytes_sent = websocket_->sendFrame(buffer.data(), buffer.size());
            auto on_error_callback = *on_error_;
            if (bytes_sent != static_cast<int>(buffer.size())) {
                on_error_callback("Failed to send full buffer. Sent " +
                        std::to_string(bytes_sent) + " of " + std::to_string(buffer.size()));
            }
        }

        WSState state() const override
        {
            return state_;
        };

        void open() override
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
            HTTPRequest request(HTTPRequest::HTTP_GET, uri_.getPath(), HTTPRequest::HTTP_1_1);
            HTTPResponse response;
            websocket_ = std::unique_ptr<WebSocket>(new WebSocket(*session_, request, response));
            if (on_open_) {
                const auto on_open_callback = *on_open_;
                on_open_callback();
            }
        }

        void close() override
        {
            if (state_ == WSState::OPEN) {
                websocket_->close();
                state_ = WSState::CLOSED;
                if (on_close_) {
                    const auto on_close_callback = *on_close_;
                    on_close_callback();
                }
            }
        }

        void reconnect() override
        {
            close();
            open();
        }

        void shutdown() override
        {
            if (state_ == WSState::OPEN) {
                websocket_->shutdown();
            }
        }

        void on_close(const HandlerFn& on_close) override
        {
            on_close_ = std::unique_ptr<HandlerFn>(new HandlerFn(on_close));
        }

        void on_error(const HandlerWithMsgFn& on_error) override
        {
            on_error_ = std::unique_ptr<HandlerWithMsgFn>(new HandlerWithMsgFn(on_error));
        }

        void on_message(const HandlerWithBufFn& on_message) override
        {
            on_message_ = std::unique_ptr<HandlerWithBufFn>(new HandlerWithBufFn(on_message));
        }

        void on_open(const HandlerFn& on_open) override
        {
            on_open_ = std::unique_ptr<HandlerFn>(new HandlerFn(on_open));
        }

    private:
        Poco::URI uri_;
        std::unique_ptr<HTTPClientSession> session_;
        std::unique_ptr<WebSocket> websocket_;
        WSState state_;

        std::unique_ptr<HandlerFn> on_open_;
        std::unique_ptr<HandlerFn> on_close_;
        std::unique_ptr<HandlerWithBufFn> on_message_;
        std::unique_ptr<HandlerWithMsgFn> on_error_;
    };

    static SSLManager* sslManager = SSLManager::instance();

}
