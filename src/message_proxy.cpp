/*
 * Copyright 017 deepstreamHub GmbH
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
#include <buffer.hpp>
#include <message_proxy.hpp>

#include <cassert>


namespace deepstream {
namespace parser {

MessageProxy::MessageProxy(
	const char* p, std::size_t offset,
	Topic topic, Action action, bool is_ack) :
	MessageProxy( p, offset, Header(topic,action,is_ack) )
{

}


MessageProxy::MessageProxy(
	const char* p, std::size_t offset, const Message::Header& header) :
	base_(p),
	offset_(offset),
	size_(header.size()),
	header_(header)
{
	assert( base_ );
}



std::size_t MessageProxy::size_impl_() const
{
	return size_;
}


const Message::Header& MessageProxy::header_impl_() const
{
	return header_;
}


std::size_t MessageProxy::num_arguments_impl_() const
{
	return arguments_.size();
}


Buffer MessageProxy::get_impl_(std::size_t i) const
{
	assert( i < arguments_.size() );

	const char* first = base_ + arguments_[i].offset();
	const char* last = first + arguments_[i].size();

	Buffer ret( first, last );

	return ret;
}


Buffer MessageProxy::to_binary_impl_() const
{
	return Buffer( base_, base_ + size_ );
}

}
}
