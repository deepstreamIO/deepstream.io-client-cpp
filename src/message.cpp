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
#include <cstring>

#include <algorithm>
#include <limits>
#include <ostream>

#include <buffer.hpp>
#include <message.hpp>

#include <cassert>


namespace deepstream
{

std::ostream& operator<<(std::ostream& os, Topic topic)
{
	os << static_cast<int>(topic);
	return os;
}


std::ostream& operator<<(std::ostream& os, Action action)
{
	os << static_cast<int>(action);
	return os;
}


std::ostream& operator<<(std::ostream& os, Sender sender)
{
	os << static_cast<int>(sender);
	return os;
}



// The list order follows from lexicographical ordering of its tokens, i.e.,
// A|A <= A|E|INVALID_AUTH_DATA <= A|REQ <= C|A ...
//
// The information about all headers is spread over the three arrays below so
// that `Message::Header::all()` can return a pair of iterators to the
// beginning and the one past the end of the list of headers.  The information
// could be kept in one array (of a struct or a tuple) but the accessing the
// information would be more involved for users of `message.hpp`.
const Message::Header HEADERS[] = {
	Message::Header( Topic::AUTH, Action::REQUEST, true ),
	Message::Header( Topic::AUTH, Action::ERROR_INVALID_AUTH_DATA ),
	Message::Header( Topic::AUTH, Action::ERROR_INVALID_AUTH_MSG ),
	Message::Header( Topic::AUTH, Action::ERROR_TOO_MANY_AUTH_ATTEMPTS ),
	Message::Header( Topic::AUTH, Action::REQUEST ),

	Message::Header( Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true ),
	Message::Header( Topic::CONNECTION, Action::CHALLENGE ),
	Message::Header( Topic::CONNECTION, Action::CHALLENGE_RESPONSE ),
	Message::Header( Topic::CONNECTION, Action::PING ),
	Message::Header( Topic::CONNECTION, Action::PONG ),
	Message::Header( Topic::CONNECTION, Action::REDIRECT ),
	Message::Header( Topic::CONNECTION, Action::REJECT ),

	Message::Header( Topic::EVENT, Action::LISTEN, true ),
	Message::Header( Topic::EVENT, Action::SUBSCRIBE, true ),
	Message::Header( Topic::EVENT, Action::UNSUBSCRIBE, true ),
	Message::Header( Topic::EVENT, Action::EVENT ),
	Message::Header( Topic::EVENT, Action::LISTEN ),
	Message::Header( Topic::EVENT, Action::SUBSCRIBE ),
	Message::Header( Topic::EVENT, Action::UNSUBSCRIBE )
};

const std::size_t NUM_HEADERS = sizeof(HEADERS) / sizeof(HEADERS[0]);

const char* HEADER_TO_STRING[] = {
	"A|A",
	"A|E|INVALID_AUTH_DATA",
	"A|E|INVALID_AUTH_MSG",
	"A|E|TOO_MANY_AUTH_ATTEMPTS",
	"A|REQ",

	"C|A",
	"C|CH",
	"C|CHR",
	"C|PI",
	"C|PO",
	"C|RED",
	"C|REJ",

	"E|A|L",
	"E|A|S",
	"E|A|US",
	"E|EVT",
	"E|L",
	"E|S",
	"E|US"
};

const std::pair<std::size_t, std::size_t> HEADER_NUM_PAYLOAD[] = {
	std::pair<std::size_t, std::size_t>(0, 1),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(1, 1),

	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(0, 1),

	std::pair<std::size_t, std::size_t>(2, 2),
	std::pair<std::size_t, std::size_t>(2, 2),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(2, 2),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(2, 2)
};


#define DS_COUNT(XS) (sizeof(XS)/sizeof(XS[0]))
static_assert( DS_COUNT(HEADERS) == DS_COUNT(HEADER_TO_STRING), "" );
static_assert( DS_COUNT(HEADERS) == DS_COUNT(HEADER_NUM_PAYLOAD), "" );


std::pair<const Message::Header*, const Message::Header*> Message::Header::all()
{
	return std::make_pair( HEADERS, HEADERS+NUM_HEADERS );
}


const char* Message::Header::to_string(Topic topic, Action action, bool is_ack)
{
	return Message::Header(topic, action, is_ack).to_string();
}


std::size_t Message::Header::size(Topic topic, Action action, bool is_ack)
{
	return Message::Header(topic, action, is_ack).size();
}


const char* Message::Header::to_string() const
{
	for(std::size_t i = 0; i < NUM_HEADERS; ++i)
	{
		if( *this == HEADERS[i] )
			return HEADER_TO_STRING[i];
	}

	assert(0);
	return nullptr;
}


std::size_t Message::Header::size() const
{
	return std::strlen( to_string() );
}



Buffer Message::Header::to_binary() const
{
	return from_human_readable( to_string() );
}



Buffer Message::from_human_readable(const char* p)
{
	return Message::from_human_readable( p, std::strlen(p) );
}

Buffer Message::from_human_readable(const char* p, std::size_t size)
{
	Buffer xs( p, p+size );
	std::replace( xs.begin(), xs.end(), '|', '\x1f' );
	std::replace( xs.begin(), xs.end(), '+', '\x1e' );

	return xs;
}


std::pair<std::size_t, std::size_t> Message::num_arguments(
	const Message::Header& header)
{
	typedef std::pair<std::size_t, std::size_t> RetType;

	for(std::size_t i = 0; i < NUM_HEADERS; ++i)
	{
		if( header == HEADERS[i] )
			return HEADER_NUM_PAYLOAD[i];
	}

	const std::size_t max = std::numeric_limits<std::size_t>::max();
	const RetType error(max, max);

	assert(0);
	return error;
}



Buffer Message::operator[] (std::size_t i) const
{
	return get_impl_(i);
}


Buffer Message::to_binary() const
{
	return to_binary_impl_();
}



std::ostream& operator<<(std::ostream& os, const Message::Header& header)
{
	os <<
		"Message::Header(" <<
		header.topic() <<
		", " <<
		header.action() <<
		(header.is_ack() ? ", true" : "") <<
		")";

	return os;
}



bool operator== (const Message::Header& left, const Message::Header& right)
{
	if( left.topic() != right.topic() ) return false;
	if( left.action() != right.action() ) return false;
	if( left.is_ack() != right.is_ack() ) return false;

	return true;
}

}
