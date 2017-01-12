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
#ifndef DEEPSTREAM_SCOPEGUARD_HPP
#define DEEPSTREAM_SCOPEGUARD_HPP

#include <functional>

#include <boost/preprocessor/cat.hpp>

#include <use.hpp>


namespace deepstream
{
	// A. Alexandrescu, P. Marginean:
	// "Generic: Change the Way You Write Exception-Safe Code -- Forever"
	// Dr. Dobbs Journal, 2000
	// www.drdobbs.com/cpp/generic-change-the-way-you-write-excepti/184403758
	struct ScopeGuard
	{
		typedef std::function<void(void)> FunT;

		ScopeGuard(FunT f) : f_(f) {}
		~ScopeGuard() { f_(); }

		FunT f_;
	};
}


#define DEEPSTREAM_ON_EXIT(...) \
	const ::deepstream::ScopeGuard BOOST_PP_CAT(deepstream_guard,__LINE__)(__VA_ARGS__); \
	::deepstream::use( BOOST_PP_CAT(deepstream_guard, __LINE__) )

#endif
