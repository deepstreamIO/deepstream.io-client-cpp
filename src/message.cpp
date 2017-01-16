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
const Message::Header HEADERS[] = {
	Message::Header( Topic::AUTH, Action::REQUEST, true ),
	Message::Header( Topic::AUTH, Action::ERROR_INVALID_AUTH_DATA ),
	Message::Header( Topic::AUTH, Action::ERROR_TOO_MANY_AUTH_ATTEMPTS ),
	Message::Header( Topic::AUTH, Action::REQUEST ),

	Message::Header( Topic::CONNECTION, Action::CHALLENGE_RESPONSE, true ),
	Message::Header( Topic::CONNECTION, Action::CHALLENGE ),
	Message::Header( Topic::CONNECTION, Action::CHALLENGE_RESPONSE ),
	Message::Header( Topic::CONNECTION, Action::REDIRECT ),
	Message::Header( Topic::CONNECTION, Action::REJECT ),

	Message::Header( Topic::EVENT, Action::LISTEN, true ),
	Message::Header( Topic::EVENT, Action::SUBSCRIBE, true ),
	Message::Header( Topic::EVENT, Action::UNSUBSCRIBE, true ),
	Message::Header( Topic::EVENT, Action::LISTEN ),
	Message::Header( Topic::EVENT, Action::SUBSCRIBE ),
	Message::Header( Topic::EVENT, Action::UNSUBSCRIBE )
};

const std::size_t NUM_HEADERS = sizeof(HEADERS) / sizeof(HEADERS[0]);

const char* HEADER_TO_STRING[] = {
	"A|A",
	"A|E|INVALID_AUTH_DATA",
	"A|E|TOO_MANY_AUTH_ATTEMPTS",
	"A|REQ",

	"C|A",
	"C|CH",
	"C|CHR",
	"C|RED",
	"C|REJ",

	"E|A|L",
	"E|A|S",
	"E|A|US",
	"E|L",
	"E|S",
	"E|US"
};

const std::pair<std::size_t, std::size_t> HEADER_NUM_PAYLOAD[] = {
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(2, 2),

	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(0, 0),
	std::pair<std::size_t, std::size_t>(1, 1),
	std::pair<std::size_t, std::size_t>(0, 1),

	std::pair<std::size_t, std::size_t>(2, 2),
	std::pair<std::size_t, std::size_t>(2, 2),
	std::pair<std::size_t, std::size_t>(1, 1),
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



std::vector<char> Message::Header::to_binary() const
{
	return from_human_readable( to_string() );
}



std::vector<char> Message::from_human_readable(const char* p)
{
	return Message::from_human_readable( p, std::strlen(p) );
}

std::vector<char> Message::from_human_readable(
	const char* p, std::size_t size)
{
	std::vector<char> xs( p, p+size );
	std::replace( xs.begin(), xs.end(), '|', '\x1f' );
	std::replace( xs.begin(), xs.end(), '+', '\x1e' );

	return xs;
}


bool operator== (const Message::Header& left, const Message::Header& right)
{
	if( left.topic() != right.topic() ) return false;
	if( left.action() != right.action() ) return false;
	if( left.is_ack() != right.is_ack() ) return false;

	return true;
}



std::pair<std::size_t, std::size_t> Message::num_arguments(
	Topic topic, Action action, bool is_ack)
{
	typedef std::pair<std::size_t, std::size_t> RetType;

	Header h(topic, action, is_ack);

	for(std::size_t i = 0; i < NUM_HEADERS; ++i)
	{
		if( h == HEADERS[i] )
			return HEADER_NUM_PAYLOAD[i];
	}

	const std::size_t max = std::numeric_limits<std::size_t>::max();
	const RetType error(max, max);

	assert(0);
	return error;
}



Message::Message(
	const char* p, std::size_t offset,
	Topic topic, Action action, bool is_ack) :
	Message( p, offset, Header(topic,action,is_ack) )
{

}


Message::Message(const char* p, std::size_t offset, const Header& header) :
	base_(p),
	offset_(offset),
	size_(header.size()),
	header_(header)
{
	assert( base_ );
}

}
