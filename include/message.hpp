#ifndef DEEPSTREAM_MESSAGE_HPP
#define DEEPSTREAM_MESSAGE_HPP


#include <cstddef>
#include <vector>


namespace deepstream
{
	struct Location
	{
		explicit Location(std::size_t offset, std::size_t length) :
			offset_(offset),
			length_(length)
		{}


		std::size_t offset_;
		std::size_t length_;
	};


	enum class Topic
	{
		AUTH,
		CONNECTION,
		ERROR,
		EVENT,
		RECORD,
		RPC
	};

	enum class Action
	{
		CREATE_OR_READ,
		ERROR_INVALID_AUTH_DATA,
		ERROR_TOO_MANY_AUTH_ATTEMPTS,
		LISTEN,
		QUERY,
		REQUEST,
		SUBSCRIBE,
		UNLISTEN,
		UNSUBSCRIBE
	};


	struct Message
	{
		typedef std::vector<Location> LocationList;

		explicit Message(
			const char* p, Topic topic, Action action, bool isAck=false);

		const char* buffer() const { return buffer_; }
		Topic topic() const { return topic_; }
		Action action() const { return action_; }
		bool isAck() const { return isAck_; }

		const char* const buffer_;
		const Topic topic_;
		const Action action_;
		const bool isAck_;

		LocationList data_;
	};
}

#endif
