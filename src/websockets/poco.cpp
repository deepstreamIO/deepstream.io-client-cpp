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
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>

#include <Poco/Exception.h>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>

#include <buffer.hpp>
#include <exception.hpp>
#include <time.hpp>
#include <websockets.hpp>
#include <websockets/poco.hpp>

#include <cassert>


namespace net = Poco::Net;


namespace deepstream {
namespace websockets {
namespace poco
{


Client::Client(const std::string& uri_string) :
	uri_( uri_string ),
	session_( uri_.getHost(), uri_.getPort() ),
	request_(net::HTTPRequest::HTTP_GET, uri_string,net::HTTPRequest::HTTP_1_1),
	websocket_(session_, request_, response_)
{
}


std::string Client::uri_impl() const
{
	return uri_.toString();
}


std::size_t Client::num_bytes_available_impl()
{
	int num_bytes = websocket_.available();
	assert( num_bytes >= 0 );

	return num_bytes;
}


time::Duration Client::get_receive_timeout_impl()
{
	Poco::Timespan t = websocket_.getReceiveTimeout();

	return std::chrono::milliseconds( t.totalMilliseconds() );
}


void Client::set_receive_timeout_impl(time::Duration t)
{
	Poco::Timespan u =
		std::chrono::duration_cast<std::chrono::microseconds>(t).count();

	websocket_.setReceiveTimeout(u);
}


std::pair<StatusCode, std::unique_ptr<Frame> > Client::receive_frame_impl()
{
	typedef std::unique_ptr<Frame> FramePtr;

	const int eof_flags =
		Frame::Bit::FIN | Frame::Opcode::CONNECTION_CLOSE_FRAME;

	int ret = 0;
	int flags = 0;

	std::size_t buffer_size = num_bytes_available();
	Buffer buffer(buffer_size, 0);

	try
	{
		ret = websocket_.receiveFrame( buffer.data(), buffer.size(), flags );
	}
	catch(Poco::TimeoutException& e)
	{
		return std::make_pair(StatusCode::NONE, nullptr);
	}
	catch(net::WebSocketException& e)
	{
		throw Exception( e.what() );
	}

	if( ret < 0 )
	{
		assert(0);
		throw std::logic_error("receiveFrame() returned a negative value");
	}

	if( ret == 0 )
	{
		return std::make_pair(StatusCode::ABNORMAL_CLOSE, nullptr);
	}

	if( flags == eof_flags && ret == 2 )
	{
		union {
			Buffer::value_type* p_data;
			std::uint16_t* p_uint16_t;
		} payload;

		payload.p_data = buffer.data();
		std::uint16_t u16 = ntohs( payload.p_uint16_t[0] );

		StatusCode status_code = (u16 == 1000)
			? StatusCode::NORMAL_CLOSE
			: StatusCode::UNKNOWN;

		return std::make_pair(
				status_code,
				FramePtr(new Frame(flags, buffer.data(), ret))
		);
	}

	return std::make_pair(
		StatusCode::NONE,
		FramePtr(new Frame(flags, buffer.data(), ret))
	);
}


int Client::send_frame_impl(const Buffer& buffer, Frame::Flags flags)
{
	int ret = websocket_.sendFrame( buffer.data(), buffer.size(), flags );

	if( ret == 0 )
		return ret;

	if( ret < 0 )
		throw SystemError(errno);

	if( ret > 0 && std::size_t(ret) < buffer.size() )
	{
		assert(0);
		throw Exception("Incomplete message sent");
	}

	return ret;
}


void Client::close_impl()
{
	websocket_.shutdown();
}

}
}
}
