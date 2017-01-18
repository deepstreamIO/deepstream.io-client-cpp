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
#ifndef DEEPSTREAM_BUFFER_HPP
#define DEEPSTREAM_BUFFER_HPP

#include <cstring>

#include <vector>

#include <cassert>


namespace deepstream
{
	// This implementation instead of a typedef allows the forward declaration
	// of Buffer. Also, the buffer can be constructed directly from strings.
	struct Buffer : public std::vector<char>
	{
		typedef std::vector<char> Base;

		Buffer() {}
		explicit Buffer(std::size_t sz) : Base(sz) {}
		Buffer(std::size_t sz, char val) : Base(sz, val) {}
		template<typename T> Buffer(T first, T last) : Base(first, last) {}
		Buffer(std::initializer_list<char> init) : Base(init) {}

		explicit Buffer(const char* p) : Base(p, p+std::strlen(p)) {assert(p);}
	};
}

#endif
