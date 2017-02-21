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
#include <exception>
#include <iostream>

#include <deepstream.hpp>

#include <Poco/Exception.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/SharedPtr.h>
#include <Poco/URI.h>
#include <Poco/URIStreamOpener.h>

int main(int argc, char* argv[])
{
    // TODO(frobware) - move all the SSL stuff into the library.

    Poco::Net::HTTPStreamFactory::registerFactory();
    Poco::Net::HTTPSStreamFactory::registerFactory();

    Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pAcceptCertHandler = new Poco::Net::AcceptCertificateHandler(true);
    Poco::Net::Context::Ptr pContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_RELAXED, 9, true, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

    Poco::Net::SSLManager::instance().initializeClient(NULL, pAcceptCertHandler, pContext);

    std::string uri = "ws://localhost:6020/deepstream";

    if (argc >= 2) {
        uri = argv[1];
    }

    try {
        deepstream::Client client(uri);

        if (client.login()) {
            std::cout << "successfully logged in to " << uri << std::endl;
            return EXIT_SUCCESS;
        } else {
            std::cerr << "failed to login to " << uri << std::endl;
            return EXIT_FAILURE;
        }
    } catch (std::exception& e) {
        std::cerr << "Caught exception \"" << e.what() << "\"" << std::endl;
        return EXIT_FAILURE;
    }
}
