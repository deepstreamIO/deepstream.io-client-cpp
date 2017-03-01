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

#include <deepstream/variant.hpp>

#include <algorithm>		// std::copy
#include <iostream>		// std::cout
#include <iterator>		// std::ostream_iterator

namespace deepstream {
  
template <class Value, class Object, class Array>
std::string json_stringify(const deepstream::Value&);

// XXX need an additional function that escapes strings.

std::string quote_str(std::string str)
{
    return "\"" + str + "\"";
}

// XXX join() - this is hideously expensive.

template <class Container>
std::string join(const Container& container, const char* delimiter = ", ")
{
    std::string s;
    const size_t n = container.size();
    for (size_t i = 0; i < n; i++) {
        s += container[i];
        if (i + 1 < n) {
            s += delimiter;
        }
    }
    return s;
}

struct json_visitor {
    std::string operator()(const int& v) const
    {
        return std::to_string(v);
    }

    std::string operator()(const double& v) const
    {
        return std::to_string(v);
    }

    std::string operator()(const bool& value) const
    {
        return value ? "true" : "false";
    }

    std::string operator()(const std::nullptr_t&) const
    {
        return "null";
    }

    std::string operator()(const std::string& value) const
    {
        return quote_str(value);
    }

    std::string operator()(const deepstream::Array& v) const
    {
        std::vector<std::string> out;

        for (const auto& x : v.values) {
            out.push_back(json_stringify<deepstream::Value, deepstream::Object, deepstream::Array>(x));
        }

        return "[" + join(out) + "]";
    }

    std::string operator()(const deepstream::Object& m) const
    {
        std::vector<std::string> out;

        for (const auto& x : m.values) {
            out.push_back(quote_str(x.first) + ": " + json_stringify<deepstream::Value, deepstream::Object, deepstream::Array>(x.second));
        }

        return "{" + join(out) + "}";
    }
};

template <class Value, class Object, class Array>
std::string json_stringify(const deepstream::Value& value)
{
    return mapbox::util::apply_visitor(json_visitor(), value);
}

std::string toJSON(const deepstream::Value& v)
{
    return json_stringify<deepstream::Value, deepstream::Object, deepstream::Array>(v);
}

}
