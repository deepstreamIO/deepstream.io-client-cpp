/*
 * Copyright 2016-2017 deepstreamHub GmbH
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
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <cstring>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include <deepstream/buffer.hpp>
#include <deepstream/message.hpp>
#include <deepstream/message_builder.hpp>
#include <deepstream/parser.hpp>
#include <deepstream/random.hpp>
#include <deepstream/scope_guard.hpp>

extern "C" {
#include <lexer.h>
}


// Remarks:
// - Do not use BOOST_CHECK_EQUAL() to compare const char* variables as
//   Boost.Test attempts to be smart by calling std::strcmp()
//   [with Boost 1.53, see boost/test/impl/test_tools.ipp:383, equal_impl()].
//   In this file, const char* variables do not always reference null-terminated
//   strings, e.g., when using std::vector<char*>::data(). If this happens,
//   Valgrind may detect invalid reads.


namespace deepstream {
    namespace parser {

        BOOST_AUTO_TEST_CASE(empty_string) {
            deepstream_parser_state state("", 0);

            int token = state.handle_token(TOKEN_EOF, "\0", 1);
            BOOST_CHECK_EQUAL(token, TOKEN_EOF);

            BOOST_CHECK(state.tokenizing_header_);
            BOOST_CHECK_EQUAL(state.offset_, 1);
            BOOST_CHECK(state.messages_.empty());
            BOOST_CHECK(state.errors_.empty());
        }


        BOOST_AUTO_TEST_CASE(simple) {
            const auto input = Message::from_human_readable("A|A+");
            const auto copy(input);
            const char *matches[] = {&input[0], &input[3], ""};
            std::size_t sizes[] = {3, 1, 1};

            const deepstream_token tokens[] = {
                    TOKEN_A_A, TOKEN_MESSAGE_SEPARATOR, TOKEN_EOF
            };

            const std::size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);


            deepstream_parser_state state(&copy[0], copy.size());

            for (std::size_t i = 0; i < num_tokens; ++i) {
                bool tokenizing_header = state.tokenizing_header_;
                BOOST_CHECK((i == 0 || i == 2) ? tokenizing_header : !tokenizing_header);

                int ret = state.handle_token(tokens[i], matches[i], sizes[i]);

                BOOST_CHECK_EQUAL(ret, tokens[i]);

                BOOST_CHECK_EQUAL(state.messages_.size(), 1);
                BOOST_CHECK(state.errors_.empty());

                std::size_t offset = std::accumulate(sizes, sizes + i + 1, 0);
                BOOST_CHECK_EQUAL(state.offset_, offset);
            }

            MessageProxy &msg = state.messages_.front();

            BOOST_CHECK(msg.base_ == copy.data());
            BOOST_CHECK_EQUAL(msg.offset_, 0);
            BOOST_CHECK_EQUAL(msg.size_, input.size());

            BOOST_CHECK_EQUAL(msg.topic(), Topic::AUTH);
            BOOST_CHECK_EQUAL(msg.action(), Action::REQUEST);
            BOOST_CHECK(msg.is_ack());

            BOOST_CHECK(msg.arguments_.empty());
        }


        BOOST_AUTO_TEST_CASE(concatenated_messages) {
            const char STRING[] = "E|L|listen+E|S|event+";

            const auto input = Message::from_human_readable(STRING);
            const auto copy(input);

            const deepstream_token tokens[] = {
                    TOKEN_E_L, TOKEN_PAYLOAD, TOKEN_MESSAGE_SEPARATOR,
                    TOKEN_E_S, TOKEN_PAYLOAD, TOKEN_MESSAGE_SEPARATOR,
                    TOKEN_EOF
            };

            const std::size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);

            const std::size_t matchlens[num_tokens] = {3, 7, 1, 3, 6, 1, 1};
            const char *matches[num_tokens] = {
                    &input[0], &input[3], &input[10],
                    &input[11], &input[14], &input[20],
                    ""
            };


            deepstream_parser_state state(&copy[0], copy.size());

            for (std::size_t i = 0; i < num_tokens; ++i) {
                BOOST_CHECK((i == 0 || i == 3 || i == 6) ?
                            state.tokenizing_header_ :
                            !state.tokenizing_header_
                );

                std::size_t offset = std::accumulate(matchlens, matchlens + i, 0);
                BOOST_CHECK_EQUAL(state.offset_, offset);

                int ret = state.handle_token(tokens[i], matches[i], matchlens[i]);

                BOOST_CHECK_EQUAL(ret, tokens[i]);

                BOOST_CHECK_EQUAL(state.messages_.size(), (i >= 3) ? 2 : 1);
                BOOST_CHECK(state.errors_.empty());

                BOOST_CHECK_EQUAL(state.offset_, offset + matchlens[i]);
            }

            BOOST_CHECK(state.tokenizing_header_);

            for (const MessageProxy &msg : state.messages_) {
                BOOST_CHECK(msg.base_ == copy.data());

                BOOST_CHECK_EQUAL(msg.topic(), Topic::EVENT);
                BOOST_CHECK(!msg.is_ack());

                BOOST_CHECK_EQUAL(msg.arguments_.size(), 1);
            }

            BOOST_CHECK_EQUAL(state.messages_.size(), 2);

            const MessageProxy &msg_f = state.messages_.front();
            BOOST_CHECK_EQUAL(msg_f.offset(), 0);
            BOOST_CHECK_EQUAL(msg_f.size(), 11);
            BOOST_CHECK_EQUAL(msg_f.topic(), Topic::EVENT);
            BOOST_CHECK_EQUAL(msg_f.action(), Action::LISTEN);

            const Location &arg_f = msg_f.arguments_.front();

            BOOST_CHECK_EQUAL(arg_f.offset_, 4);
            BOOST_CHECK_EQUAL(arg_f.size_, 6);
            BOOST_CHECK(!strncmp(&input[arg_f.offset_], "listen", arg_f.size_));


            const MessageProxy &msg_b = state.messages_.back();
            BOOST_CHECK_EQUAL(msg_b.offset(), 11);
            BOOST_CHECK_EQUAL(msg_b.size(), 10);
            BOOST_CHECK_EQUAL(msg_b.topic(), Topic::EVENT);
            BOOST_CHECK_EQUAL(msg_b.action(), Action::SUBSCRIBE);

            const Location &arg_b = msg_b.arguments_.front();

            BOOST_CHECK_EQUAL(arg_b.offset_, 15);
            BOOST_CHECK_EQUAL(arg_b.size_, 5);
            BOOST_CHECK(!strncmp(&input[arg_b.offset_], "event", arg_b.size_));
        }


        BOOST_AUTO_TEST_CASE(invalid_number_of_arguments) {
            const char STRING[] = "E|A|L|x|y+";

            const auto input = Message::from_human_readable(STRING);
            const auto copy(input);

            const deepstream_token TOKENS[] = {
                    TOKEN_E_A_L, TOKEN_PAYLOAD, TOKEN_PAYLOAD, TOKEN_MESSAGE_SEPARATOR,
                    TOKEN_EOF
            };

            const std::size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

            const std::size_t MATCHLENS[NUM_TOKENS] = {5, 2, 2, 1, 1};
            const char *MATCHES[NUM_TOKENS] = {
                    &input[0], &input[5], &input[7], &input[9], ""
            };


            deepstream_parser_state state(&copy[0], copy.size());

            for (std::size_t i = 0; i < NUM_TOKENS; ++i) {
                bool tokenizing_header = state.tokenizing_header_;
                BOOST_CHECK((i == 0 || i == 4) ? tokenizing_header : !tokenizing_header);

                std::size_t offset = std::accumulate(MATCHLENS, MATCHLENS + i, 0);
                BOOST_CHECK_EQUAL(state.offset_, offset);

                int ret = state.handle_token(TOKENS[i], MATCHES[i], MATCHLENS[i]);

                BOOST_CHECK_EQUAL(ret, TOKENS[i]);

                BOOST_CHECK_EQUAL(state.messages_.size(), (i >= 3) ? 0 : 1);
                BOOST_CHECK_EQUAL(state.errors_.size(), (i >= 3) ? 1 : 0);

                BOOST_CHECK_EQUAL(state.offset_, offset + MATCHLENS[i]);
            }


            const Error &e = state.errors_.front();

            BOOST_CHECK_EQUAL(e.location_.offset_, 0);
            BOOST_CHECK_EQUAL(e.location_.size_, 10);
            BOOST_CHECK_EQUAL(e.tag_, Error::INVALID_NUMBER_OF_ARGUMENTS);
        }


// lexer, parser integration tests

        BOOST_AUTO_TEST_CASE(simple_integration) {
            const char raw[] = "A|A+ERROR+++E|A|L|p+";
            const std::vector<char> input = Message::from_human_readable(raw);

            std::vector<char> lexer_input(input.size() + 2);
            std::copy(input.cbegin(), input.cend(), lexer_input.begin());

            yyscan_t scanner;
            int ret = yylex_init(&scanner);
            BOOST_REQUIRE_EQUAL(ret, 0);
            DEEPSTREAM_ON_EXIT([&scanner]() {
                yylex_destroy(scanner);
            });

            YY_BUFFER_STATE buffer = \
        yy_scan_buffer(lexer_input.data(), lexer_input.size(), scanner);
            yy_switch_to_buffer(buffer, scanner);

            State parser(input.data(), input.size());
            yyset_extra(&parser, scanner);

            for (int ret = TOKEN_UNKNOWN; ret != 0; ret = yylex(scanner));

            BOOST_CHECK(parser.buffer_ == input.data());
            BOOST_CHECK_EQUAL(parser.buffer_size_, input.size());

            BOOST_CHECK(parser.tokenizing_header_);
            BOOST_CHECK_EQUAL(parser.offset_, parser.buffer_size_ + 1);

            BOOST_CHECK_EQUAL(parser.messages_.size(), 2);
            BOOST_CHECK_EQUAL(parser.errors_.size(), 1);

            const MessageProxy &msg_f = parser.messages_.front();
            BOOST_CHECK(msg_f.base() == input.data());
            BOOST_CHECK_EQUAL(msg_f.offset(), 0);
            BOOST_CHECK_EQUAL(msg_f.size(), 4);
            BOOST_CHECK_EQUAL(msg_f.topic(), Topic::AUTH);
            BOOST_CHECK_EQUAL(msg_f.action(), Action::REQUEST);
            BOOST_CHECK(msg_f.is_ack());
            BOOST_CHECK_EQUAL(msg_f.num_arguments(), 0);

            const MessageProxy &msg_b = parser.messages_.back();
            BOOST_CHECK(msg_b.base() == input.data());
            BOOST_CHECK_EQUAL(msg_b.offset(), 12);
            BOOST_CHECK_EQUAL(msg_b.size(), 8);
            BOOST_CHECK_EQUAL(msg_b.topic(), Topic::EVENT);
            BOOST_CHECK_EQUAL(msg_b.action(), Action::LISTEN);
            BOOST_CHECK(msg_b.is_ack());
            BOOST_CHECK_EQUAL(msg_b.num_arguments(), 1);

            const Error &error = parser.errors_.front();
            BOOST_CHECK_EQUAL(error.location().offset(), 4);
            BOOST_CHECK_EQUAL(error.location().size(), 8);
            BOOST_CHECK_EQUAL(error.tag(), Error::UNEXPECTED_TOKEN);
        }


        BOOST_AUTO_TEST_CASE(random_messages) {
            for (std::size_t iteration = 0; iteration < 50; ++iteration) {
                random::Engine engine(iteration);

                std::vector<Message::Header> headers;
                Buffer input;

                for (std::size_t i = 0; i < 20; ++i) {
                    MessageBuilder message_builder = random::make_message(&engine);

                    Buffer bin = message_builder.to_binary();
                    input.insert(input.end(), bin.cbegin(), bin.cend());

                    headers.push_back(message_builder.header());
                }

                input.push_back('\0');
                input.push_back('\0');

                auto ret = execute(input.data(), input.size());

                const ErrorList &errors = ret.second;
                BOOST_CHECK(errors.empty());

                const MessageList &messages = ret.first;

                BOOST_REQUIRE_EQUAL(messages.size(), headers.size());

                auto i = headers.cbegin();
                auto j = messages.cbegin();
                for (; i != headers.cend(); ++i, ++j)
                    BOOST_CHECK_EQUAL(*i, j->header());
            }
        }


        BOOST_AUTO_TEST_CASE(all_messages_random_order) {
            auto all_ret = Message::Header::all();

            for (std::size_t iteration = 0; iteration < 100; ++iteration) {
                random::Engine engine(iteration);

                Buffer input;
                std::vector<Message::Header> headers(all_ret.first, all_ret.second);
                std::shuffle(headers.begin(), headers.end(), engine);

                for (const Message::Header &h : headers) {
                    MessageBuilder message = random::make_message(&engine, h);

                    Buffer bin = message.to_binary();
                    input.insert(input.end(), bin.cbegin(), bin.cend());
                }


                input.push_back('\0');
                input.push_back('\0');

                auto ret = execute(input.data(), input.size());

                const ErrorList &errors = ret.second;
                BOOST_CHECK(errors.empty());

                const MessageList &messages = ret.first;

                BOOST_REQUIRE_EQUAL(messages.size(), headers.size());

                auto i = headers.cbegin();
                auto j = messages.cbegin();
                for (; i != headers.cend(); ++i, ++j)
                    BOOST_CHECK_EQUAL(*i, j->header());
            }
        }

    }
}
