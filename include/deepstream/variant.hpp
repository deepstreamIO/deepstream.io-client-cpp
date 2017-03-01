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

#ifndef DEEPSTREAM_VARIANT_HPP
#define DEEPSTREAM_VARIANT_HPP

#include <mapbox/variant.hpp>
#include <mapbox/variant_io.hpp>

#include <unordered_map>
#include <vector>

namespace deepstream {

struct Array;
struct Object;

using String = std::string;

using Value = mapbox::util::variant<
    int,
    double,
    bool,
    std::nullptr_t,
    std::string,
    mapbox::util::recursive_wrapper<Array>,
    mapbox::util::recursive_wrapper<Object> >;

struct Array {
    std::vector<Value> values;
};

struct Object {
    std::unordered_map<std::string, Value> values;
};

std::string toJSON(const Value&);

}

#endif
