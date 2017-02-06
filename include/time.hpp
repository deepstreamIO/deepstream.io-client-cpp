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
#ifndef DEEPSTREAM_TIME_HPP
#define DEEPSTREAM_TIME_HPP

#include <chrono>


namespace deepstream
{
	/**
	 * The aliases in this module should ease the use of monotonic clocks within
	 * deepstream.
	 */
	namespace time
	{
		typedef std::chrono::steady_clock Clock;
		typedef Clock::duration Duration;
	}
}

#endif
