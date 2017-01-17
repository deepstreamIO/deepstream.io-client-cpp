# C++ Client Proposal

## Requirements

The deepstream C++ client shall
- be licensed under the Apache License, Version 2.0,
- provide a C interface,
- work on POSIX-compliant systems,
- work on multiple architectures including x86, x86-64, and ARMv7,
- support unicode (UTF-8),
- implement a deepstream message parser,
- implement a deepstream client state machine,
- implement deepstream authentication,
- implement deepstream ping-pong messages,
- implement deepstream events before records and remote procedure calls (RPCs),
- use an existing websockets implementation,
- separate message parser and state machine from the websockets implementation and timer mechanism,
- provide a default timer mechanism,
- contain a demo program (connect with server, log in, receive event),
- use a popular build system,
- provide automated tests,
- provide documentation,
- provide continuous integration setups for Linux, MacOS, and Windows,
- pass the [Cucumber client specs](https://github.com/deepstreamIO/deepstream.io-client-specs) tests,
- pass the [deepstream E2E tests](https://github.com/deepstreamIO/deepstream.io-e2e) tests after the client specs.



## Guidelines

- The implementation has tightly bounded memory consumption.
- C++14 should be avoided if possible.
- Build times are kept short.



## Implementation Proposal

The deepstream C++ client
- will be implemented in C++11 as far as possible,
- uses [uWebSockets](https://github.com/uWebSockets/uWebSockets) as Websockets implementation,
- provides a timer mechanism using POSIX threads and `timer_create()`,
- uses CMake as build system,
- provides unit tests using Boost.Test,
- is tested with Valgrind,
- will be developed with the aid of a static analyzer,
- provides high-level documentation on deepstream.io if necessary,
- provides source-level doumentation with Doxygen,
- will decide later on a JSON parser.



## Schedule

Implementing implies static analysis and testing on at least one platform.
Testing includes unit tests but not necessarily Cucumber tests.
- Implement parser
- Implement state machine
- Integrate websockets functionality
- Implement default timer mechanism
- Implement deepstream connect
- Implement deepstream heartbeat
- Pass Cucumber login tests
- Implement deepstream events
- Pass Cucumber event tests
- Start testing on multiple platforms the latest here
- Test performance
- Integrate JSON functionality
- Test unicode functionality
- Implement deepstream records
- Pass Cucuber record tests
- Implement deepstream RPCs
- Pass Cucumber RPC tests
- Implement deepstream presence feature
- Pass Cucumber presence tests



## Relevant Links

### deepstream

- [Client states](https://deepstream.io/docs/common/constants/#connection-states))
- [deepstream server message parser](https://github.com/deepstreamIO/deepstream.io-ws)
- Thread: [_deepstream.io client for C/C++_](https://github.com/deepstreamIO/deepstream.io/issues/69)
- Tutorial: [_Writing a Client_](https://deepstream.io/tutorials/core/writing-a-client/)
- [Cenacle deepstream C++ client](https://github.com/CenacleResearch/deepstream-cpp-client)
- [Autobahn benchmark](http://autobahn.ws)
- [Java test application](https://github.com/deepstreamIO/deepstream.io-client-java/blob/master/src/testapplication/java/io/deepstream/testapp/Subscriber.java)
- [deepstream.io event-handler.js](https://github.com/deepstreamIO/deepstream.io-client-js/blob/f7ac3a3aaed269ba37185241e96341339771aa96/src/event/event-handler.js#L165)


### Static Analysis Links

- Clang Static Analyzer
- http://dsw.users.sonic.net/oink/index.html
- https://github.com/mre/awesome-static-analysis


### JSON Parsers

Consider JSON paths support.

- [Jansson](http://www.digip.org/jansson/)
	- full Unicode support (UTF-8)
	- MIT license
	- last commit: Oct 2016
- [POCO](https://pocoproject.org)
	- Boost software license
	- last commit: Dec 2016
- [taocpp/JSON](https://github.com/taocpp/json)
	- fully standards compliant (100% in Native JSON Benchmark)
	- header only
	- MIT license
	- last commit: Dec 2016
- [RapidJSON](https://github.com/miloyip/rapidjson)
	- object-oriented: `Document d; d.Parse(json);`
	- header only
	- MIT license(?)
	- last commit: Dec 2016
- [jsmn](http://zserge.com/jsmn.html)
	- deserialize only
- [cJSON](https://github.com/DaveGamble/cJSON)
	- 2300 LOC C89
- [JSON-C](https://github.com/json-c/json-c)
	- MIT license
- [NXJSON](https://bitbucket.org/yarosla/nxjson)
	- non-validating
- [yajl](https://lloyd.github.io/yajl/)
	- "event-driven (SAX-style)"
	- object-oriented C, stream parsing,
	- ISC license


### WebSocket Libraries

GitHub statistics as of Jan 17, 2017.

- [Beast HTTP/WebSocket](https://github.com/vinniefalco/Beast)
	- depends on boost>=1.58
	- uses Boost.Asio
	- header only
	- GitHub: 54 watchers, 265 stars, 52 forks
	- Boost license
- [Boost.Asio](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
	- general network programming
	- Boost license
- [libwebsockets](https://github.com/warmcat/libwebsockets)
	- C
	- RFC 7692 support (Compression Extensions for WebSockets)
	- GitHub: 147 watchers, 982 stars, 436 forks
	- LGPL 2.1
- [noPoll](http://aspl.es/nopoll/)
	- LGPL 2.1
- [POCO WebSockets](https://pocoproject.org/docs/Poco.Net.WebSocket.html)
	- Boost license
- [WebSocket++](https://github.com/zaphoyd/websocketpp)
	- uses C++11 or Boost.Asio
	- header only
	- GitHub: 207 watchers, 1564 stars, 559 forks
	- "full RFC6455 support"
	- 3-clause BSD license


### Testing

- Continuous Integration
- Cucumber
- Fuzzing
	- zzuf
	- American fuzzy lop
	- https://github.com/aoh/radamsa
	- https://www.owasp.org/index.php/Fuzzing
