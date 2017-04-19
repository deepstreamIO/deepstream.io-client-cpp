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

#include "deepstream/lib/json.hpp"
#include "deepstream/lib/json-handler.hpp"

#include "test/utils.hpp"

namespace deepstream {

struct FailHandler : public ErrorHandler {
    virtual void on_error(const std::string &) override
    {
        BOOST_FAIL("There should be no errors");
    }
};

struct CheckHandler : public ErrorHandler {
    void on_error(const std::string &) override
    {
        error_count++;
    }

    void reset()
    {
        error_count = 0;
    }

    unsigned long error_count = 0;
};

BOOST_AUTO_TEST_CASE(deserialize_prefixed)
{
    FailHandler failh;
    JSONHandler json_handler(failh);

    CheckHandler checkh;
    JSONHandler json_handler_check_errors(checkh);

    // null
    {
        const auto result = json_handler_check_errors.prefixed_to_json(Buffer(""));
        BOOST_CHECK(result.is_null());
        BOOST_CHECK(checkh.error_count > 0);
        checkh.reset();
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("L"));
        BOOST_CHECK(result.is_null());
    }

    // undefined
    {
        const auto result = json_handler.prefixed_to_json(Buffer("U"));
        BOOST_CHECK(result.is_null());
    }

    // boolean
    {
        const auto result = json_handler.prefixed_to_json(Buffer("T"));
        BOOST_CHECK(result.is_boolean());
        BOOST_CHECK_EQUAL(result, json(true));
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("F"));
        BOOST_CHECK(result.is_boolean());
        BOOST_CHECK_EQUAL(result, json(false));
    }

    // unexpected payload
    {
        const auto result = json_handler.prefixed_to_json(Buffer("Tunexpected payload"));
        BOOST_CHECK(result.is_boolean());
        BOOST_CHECK_EQUAL(result, json(true));
    }

    // string
    {
        const auto result = json_handler.prefixed_to_json(Buffer("S"));
        BOOST_CHECK(result.is_string());
        BOOST_CHECK_EQUAL(result, json(""));
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("Sfoo"));
        BOOST_CHECK(result.is_string());
        BOOST_CHECK_EQUAL(result, json("foo"));
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("S[1, 2, 3]"));
        BOOST_CHECK(result.is_string());
        BOOST_CHECK_EQUAL(result, json("[1, 2, 3]"));
    }

    // object
    {
        const auto result = json_handler_check_errors.prefixed_to_json(Buffer("O"));
        BOOST_CHECK(result.is_null());
        BOOST_CHECK(checkh.error_count > 0);
        checkh.reset();
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("O{}"));
        BOOST_CHECK(result.is_object());
        BOOST_CHECK_EQUAL(result, json({}));
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("O[1, 2, 3]"));
        BOOST_CHECK(result.is_array());
        BOOST_CHECK_EQUAL(result, json({1,2,3}));
    }
    {
        const auto result = json_handler.prefixed_to_json(
                Buffer("O{\"a\": {\"nested\": [\"type\", 2, 3.5, false]}}"));
        BOOST_CHECK(result.is_object());
        BOOST_CHECK_EQUAL(result, json(
                    { { "a", { { "nested", { "type", 2, 3.5, false } } } } }));
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("Onull"));
        BOOST_CHECK(result.is_null());
    }

    // number
    {
        const auto result = json_handler_check_errors.prefixed_to_json(Buffer("N"));
        BOOST_CHECK(result.is_null());
        BOOST_CHECK(checkh.error_count > 0);
        checkh.reset();
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("N234"));
        BOOST_CHECK(result.is_number());
        BOOST_CHECK_EQUAL(result, json(234));
    }
    {
        const auto result = json_handler.prefixed_to_json(Buffer("N2.3"));
        BOOST_CHECK(result.is_number());
        BOOST_CHECK_EQUAL(result, json(2.3));
    }
    {
        const auto result = json_handler_check_errors.prefixed_to_json(Buffer("N.2.3"));
        BOOST_CHECK(result.is_null());
        BOOST_CHECK(checkh.error_count > 0);
        checkh.reset();
    }
}

BOOST_AUTO_TEST_CASE(serialize_prefixed)
{
    FailHandler failh;
    JSONHandler json_handler(failh);

    // null
    {
        const auto result = json_handler.to_prefixed_buffer(json(nullptr));
        BOOST_CHECK_EQUAL(result, Buffer("L"));
    }

    // boolean
    {
        const auto result = json_handler.to_prefixed_buffer(json(true));
        BOOST_CHECK_EQUAL(result, Buffer("T"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json(false));
        BOOST_CHECK_EQUAL(result, Buffer("F"));
    }

    // string
    {
        const auto result = json_handler.to_prefixed_buffer(json(""));
        BOOST_CHECK_EQUAL(result, Buffer("S"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json("ps\"st"));
        BOOST_CHECK_EQUAL(result, Buffer("Sps\"st"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json("{}"));
        BOOST_CHECK_EQUAL(result, Buffer("S{}"));
    }

    // object
    {
        const auto result = json_handler.to_prefixed_buffer(json({}));
        BOOST_CHECK_EQUAL(result, Buffer("O{}"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json::array());
        BOOST_CHECK_EQUAL(result, Buffer("O[]"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json(
                    { { "deeply", "nested" }, { "json", { "object", 2, 3.0, true, nullptr } } }));
        BOOST_CHECK_EQUAL(result, Buffer(
                    "O{\"deeply\":\"nested\",\"json\":[\"object\",2,3.0,true,null]}"));
    }

    // number
    {
        const auto result = json_handler.to_prefixed_buffer(json(1));
        BOOST_CHECK_EQUAL(result, Buffer("N1"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json(2.34));
        BOOST_CHECK_EQUAL(result, Buffer("N2.34"));
    }
    {
        const auto result = json_handler.to_prefixed_buffer(json(.23));
        BOOST_CHECK_EQUAL(result, Buffer("N0.23"));
    }
}

}
