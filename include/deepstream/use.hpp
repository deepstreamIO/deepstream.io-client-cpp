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
#ifndef DEEPSTREAM_USE_HPP
#define DEEPSTREAM_USE_HPP

namespace deepstream {
    /**
     * Helper function used for suppressing "unused variable" warnings.
     *
     * Warnings about unused variables are useful but in some situations there
     * are false positives, e.g., when variables are only used during debugging
     * and one is compiling in release mode (here: with `NDEBUG` defined):
     *
     *     int ret = function(args); // causes warning: unused variable `ret'
     *     assert( ret == 0 );
     *     return;
     */
    template<typename T>
    void use(const T &) {}
}

#endif
