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

#include <climits>

#include <algorithm>
#include <numeric>

#include <deepstream/buffer.hpp>
#include <deepstream/message.hpp>


namespace deepstream
{

BOOST_AUTO_TEST_CASE(from_human_readable_empty)
{
	auto xs = Message::from_human_readable("");
	BOOST_CHECK_EQUAL( xs.size(), 0 );

	auto ys = Message::from_human_readable("", 0);
	BOOST_CHECK_EQUAL( xs.size(), 0 );
}


BOOST_AUTO_TEST_CASE(from_human_readable_simple)
{
	std::vector<char> input(CHAR_MAX, 0);
	std::iota( input.begin(), input.end(), 0 );

	Buffer output = Message::from_human_readable( input.data(), input.size() );

	BOOST_CHECK_EQUAL( input.size(), output.size() );

	for(std::size_t i = 0; i < input.size(); ++i)
	{
		char c = input[i];
		char out = (c=='+') ? 0x1e : ((c=='|') ? 0x1f : c);

		BOOST_CHECK_EQUAL( output[i], out );
	}
}


BOOST_AUTO_TEST_CASE(from_human_readable_nop)
{
	std::vector<char> input(CHAR_MAX, 0);
	std::iota( input.begin(), input.end(), 0 );

	input['+'] = 'X';
	input['|'] = 'X';

	Buffer output = Message::from_human_readable( input.data(), input.size() );

	BOOST_CHECK_EQUAL( input.size(), output.size() );
	BOOST_CHECK( std::equal(input.begin(), input.end(), output.begin()) );
}


}
