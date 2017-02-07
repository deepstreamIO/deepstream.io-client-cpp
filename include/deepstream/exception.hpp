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
#ifndef DEEPSTREAM_EXCEPTION_HPP
#define DEEPSTREAM_EXCEPTION_HPP

#include <stdexcept>
#include <string>


namespace deepstream
{
	/**
	 * An exception class whose main purpose is to ease recognition of unhandled
	 * error cases.
	 *
	 * The idea is to use `Exception` throughout the code, catching and wrapping
	 * exceptions from external code as needed. Consequently, a caught exception
	 * which is not of type `Exception` or of type `std::system_error` is an
	 * unhandled error condition.
	 */
	struct Exception : public std::runtime_error
	{
		explicit Exception(const std::string&);
	};
}

#endif
