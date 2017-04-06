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
#include <Poco/Timespan.h>
#include <Poco/URI.h>

#include <deepstream/lib/poco-ws.hpp>

#include <algorithm> // std::max

#ifndef NDEBUG
#include <iostream>
#define DEBUG_MSG(str) do { std::cout << "# "<< str << std::endl; } while( false )
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
    {
    }

    PocoWSHandler::~PocoWSHandler()
    {
    }

    void PocoWSHandler::process_messages()
    {
        if (state_ != WSState::OPEN) {
            return;
        }

        if (!next_read_non_blocking()) {
            return;
        }

        // WebSocket.available always returns 0 on secure websockets
        // see https://github.com/pocoproject/poco/issues/1482

        // If a frame exceeds the buffer size, a WebSocket Exception is thrown
        // and the socket must be discarded, so best to declare large enough!
        // The development branch of Poco supports use of the dynamic
        // `Poco::Buffer` for `receiveFrame`, but this was not in the most recent
        // release (1.7.8)

        const std::size_t bytes_available = websocket_->available();
        const std::size_t min_buffer_size = 1024;
        const std::size_t buffer_size = std::max(bytes_available, min_buffer_size);

        Buffer buffer(buffer_size, 0);

        int offset = 0;
        int bytes_read = 0;
        do {
            bytes_read = read_frame(buffer, offset);
            offset += bytes_read;
        } while (bytes_read != 0 && next_read_non_blocking());

        if (offset == 0) {
            return;
        }

        buffer.resize(offset);

        (*on_message_)(std::move(buffer));
    }

    bool PocoWSHandler::next_read_non_blocking()
    {
        return websocket_->poll(Poco::Timespan(), Socket::SelectMode::SELECT_READ);
    }

    int PocoWSHandler::read_frame(Buffer &buffer, int offset)
    {
        int bytes_received = 0;
        int flags = 0;

        try {
            bytes_received = websocket_->receiveFrame(
                    buffer.data() + offset, buffer.size() - offset, flags);
        } catch (Poco::TimeoutException &e) {
            assert(websocket_->secure());

            // when using wss, occasionally we may read from a socket that
            // looks readable, but does not have sufficient data to decrypt
            // this will cause a block (so sendTimeout is short)
            // see https://github.com/pocoproject/poco/issues/1482
            DEBUG_MSG("Socket read timeout");
            return 0;
        } catch (WebSocketException &e) {
            state(WSState::ERROR);
            (*on_error_)(e.displayText());
            return 0;
        } catch (NetException &e) {
            state(WSState::ERROR);
            (*on_error_)(e.displayText());
            return 0;
        }

        if (bytes_received == 0) {
            DEBUG_MSG("read zero bytes from websocket... closing");
            state(WSState::CLOSED);
            close();
            return 0;
        }

        const int eof_flags = WebSocket::FrameFlags::FRAME_FLAG_FIN
                            | WebSocket::FrameOpcodes::FRAME_OP_CLOSE;

        if (flags == eof_flags) {
            state(WSState::CLOSED);
            (*on_close_)();
            return 0;
        }

        return bytes_received;
    }

    std::string PocoWSHandler::URI() const
    {
        return uri_.toString();
    }

    void PocoWSHandler::URI(std::string uri)
    {
        uri_ = uri;
        if (state_ == WSState::OPEN) {
            open();
        }
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
            return false;
        }
        return true;
    }

    void PocoWSHandler::state(const WSState state)
    {
        DEBUG_MSG("Websocket State Changed: " << static_cast<int>(state));
        state_ = state;
    }

    void PocoWSHandler::open()
    {
        DEBUG_MSG("<> Connecting to \"" << uri_.toString() << "\" <>");
        bool initialised = on_message_ && on_error_ && on_open_ && on_close_;
        if (!initialised) {
            throw std::runtime_error("Unable to open websocket: not all handlers have been set");
        }

        if (uri_.empty()) {
            throw std::runtime_error("Unable to open websocket: no URI is set");
        }
        try {
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
            // 100us blocking read timeout (such blocking reads should be rare)
            websocket_->setReceiveTimeout(Poco::Timespan(0, 100));
        } catch (NetException &e) {
            (*on_error_)(e.displayText());
        }
        state(WSState::OPEN);
        (*on_open_)();
    }

    void PocoWSHandler::close()
    {
        if (state_ == WSState::OPEN) {
            websocket_->close();
            state(WSState::CLOSED);
        }
        websocket_ = nullptr;
        (*on_close_)();
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
    static SSLManager* sslManager = SSLManager::instance();

}
