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
#include <cerrno>
#include <cstdint>

#include <algorithm>
#include <system_error>

#include <Poco/Exception.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Timespan.h>

#include <deepstream/buffer.hpp>
#include <deepstream/exception.hpp>
#include <deepstream/time.hpp>
#include <deepstream/websockets.hpp>
#include <deepstream/websockets/poco.hpp>

#include <cassert>

namespace net = Poco::Net;

namespace deepstream {
namespace websockets {

    namespace poco {
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

        private:
            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pAcceptCertHandler_;
            Poco::Net::Context::Ptr pContext_;
        };

        Client* Client::makeClient(const std::string& uri_string)
        {
            Poco::URI uri(uri_string);
            Poco::Net::HTTPClientSession* session;

            if (uri.getScheme() == "wss") {
                session = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
            } else {
                session = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
            }

            return new websockets::poco::Client(uri_string, std::shared_ptr<Poco::Net::HTTPClientSession>(session));
        }

      Client::Client(const std::string& uri, std::shared_ptr<Poco::Net::HTTPClientSession> session) try
            : uri_(uri),
              request_(net::HTTPRequest::HTTP_GET, uri_.getPath(), net::HTTPRequest::HTTP_1_1),
              websocket_(*session, request_, response_) {
        } catch (Poco::Exception& e) {
            throw Exception(e.displayText());
        }

        std::size_t Client::num_bytes_available()
        {
            int num_bytes = websocket_.available();
            assert(num_bytes >= 0);

            return num_bytes;
        }

        std::shared_ptr<websockets::Client>
        Client::construct_impl(const std::string& uri) const
        {
            return std::shared_ptr<websockets::Client>(Client::makeClient(uri));
        }

        std::string Client::uri_impl() const { return uri_.toString(); }

        time::Duration Client::get_receive_timeout_impl()
        {
            Poco::Timespan t = websocket_.getReceiveTimeout();

            return std::chrono::milliseconds(t.totalMilliseconds());
        }

        void Client::set_receive_timeout_impl(time::Duration t)
        {
            Poco::Timespan u = std::chrono::duration_cast<std::chrono::microseconds>(t).count();

            websocket_.setReceiveTimeout(u);
        }

        std::pair<State, std::unique_ptr<Frame> > Client::receive_frame_impl()
        {
            typedef std::unique_ptr<Frame> FramePtr;

            const int eof_flags = Frame::Bit::FIN | Frame::Opcode::CONNECTION_CLOSE_FRAME;

            int ret = 0;
            int flags = 0;

            const std::size_t min_buffer_size = 256;
            std::size_t buffer_size = std::max(num_bytes_available(), min_buffer_size);
            Buffer buffer(buffer_size, 0);

            try {
                ret = websocket_.receiveFrame(buffer.data(), buffer.size(), flags);
            } catch (Poco::TimeoutException& e) {
                return std::make_pair(State::OPEN, nullptr);
            } catch (net::WebSocketException& e) {
                throw Exception(e.displayText());
            }

            if (ret < 0) {
                assert(0);
                throw std::logic_error("receiveFrame() returned a negative value");
            }

            if (ret == 0) {
                return std::make_pair(State::CLOSED, nullptr);
            }

            if (flags == eof_flags && ret != 2) {
                return std::make_pair(State::ERROR,
                    FramePtr(new Frame(flags, buffer.data(), ret)));
            }

            if (flags == eof_flags && ret == 2) {
                return std::make_pair(State::CLOSED,
                    FramePtr(new Frame(flags, buffer.data(), ret)));
            }

            return std::make_pair(State::OPEN,
                FramePtr(new Frame(flags, buffer.data(), ret)));
        }

        State Client::send_frame_impl(const Buffer& buffer, Frame::Flags flags)
        {
            int ret = websocket_.sendFrame(buffer.data(), buffer.size(), flags);

            if (ret == 0)
                return State::CLOSED;

            if (ret < 0)
                throw std::system_error(errno, std::system_category());

            if (ret > 0 && std::size_t(ret) < buffer.size()) {
                assert(0);
                throw Exception("Incomplete message sent");
            }

            return State::OPEN;
        }

        void Client::close_impl() { websocket_.shutdown(); }

      //static std::shared_ptr<poco::SSLManager> sslManager(new SSLManager());
    }
}
}
