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
#ifndef DEEPSTREAM_PARSER_H
#define DEEPSTREAM_PARSER_H

/**
 * @file
 *
 * This C header file glues the C scanner code in with the C++ parser.
 */

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#if __cplusplus
extern "C" {
#endif

struct deepstream_parser_state;

/**
 * This enumeration contains all tokens that are recognized by the scanner.
 *
 * The use of `UCHAR_MAX` below is motivated by the functions in, e.g.,
 * `ctype.h` which assume char values in the interval [0, 255]. GNU Bison uses
 * a similar numbering scheme for its tokens.
 */
enum deepstream_token {
    TOKEN_EOF = INT_MIN,
    TOKEN_UNKNOWN = UCHAR_MAX + 1,
    TOKEN_PAYLOAD,
    TOKEN_MESSAGE_SEPARATOR,

    TOKEN_A_A,
    TOKEN_A_E_IAM, // A|E|INVALID_AUTH_MSG
    TOKEN_A_E_IAD, // A|E|INVALID_AUTH_DATA
    TOKEN_A_E_TMAA, // A|E|TOO_MANY_AUTH_ATTEMPTS
    TOKEN_A_REQ,

    TOKEN_C_A,
    TOKEN_C_CH,
    TOKEN_C_CHR,
    TOKEN_C_PI,
    TOKEN_C_PO,
    TOKEN_C_RED,
    TOKEN_C_REJ,

    TOKEN_E_A_L,
    TOKEN_E_A_S,
    TOKEN_E_A_US,
    TOKEN_E_EVT,
    TOKEN_E_L,
    TOKEN_E_LA,
    TOKEN_E_LR,
    TOKEN_E_S,
    TOKEN_E_SP,
    TOKEN_E_SR,
    TOKEN_E_US,

    TOKEN_U_A_S,
    TOKEN_U_A_US,
    TOKEN_U_PNJ,
    TOKEN_U_PNL,
    TOKEN_U_Q,
    TOKEN_U_S,
    TOKEN_U_US,
    /**
     * The following token is a dummy value for development purposes, e.g.,
     * the number of valid tokens is TOKENS_MAXVAL - TOKEN_UNKNOWN + 1.
     */
    TOKEN_MAXVAL
};

bool is_header_token(enum deepstream_token);

/**
 * The callback handed to the scanner.
 */
int deepstream_parser_handle(struct deepstream_parser_state*,
    enum deepstream_token, const char*, size_t);

/**
 * This macro either
 * - returns the last read token for testing purposes, or
 * - executes the parser callback.
 */
#ifdef DEEPSTREAM_TEST_LEXER
#define DS_PARSE(TOKEN) (TOKEN)
#else
#define DS_PARSE(TOKEN)                                     \
    deepstream_parser_handle(yyget_extra(yyscanner), TOKEN, \
        yyget_text(yyscanner), yyget_leng(yyscanner))
#endif

#if __cplusplus
}
#endif

#endif
