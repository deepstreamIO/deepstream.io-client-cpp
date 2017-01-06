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
#include <message.hpp>

#include <cassert>


namespace deepstream
{

Message::Message(
	const char* p, std::size_t offset, std::size_t header_size,
	Topic topic, Action action, bool isAck) :
	base_(p),
	offset_(offset),
	size_(header_size),
	topic_(topic),
	action_(action),
	isAck_(isAck)
{
	assert(base_);
	assert(header_size > 0);
}

}
