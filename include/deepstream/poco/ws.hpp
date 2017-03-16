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

#pragma once

#include <deepstream/core/ws.hpp>
#include <iostream>

class PocoWSFactory : public deepstream::WSFactory {
public:
  deepstream::WS* connect(const std::string& uri);
  static PocoWSFactory* instance();
public:
  PocoWSFactory() {
    std::cout << "hmm" << std::endl;
  }
  ~PocoWSFactory() {
    std::cout << "~hmm" << std::endl;
  }
};
