#ifndef DEEPSTREAM_PARSER_HPP
#define DEEPSTREAM_PARSER_HPP

#include <cstddef>

#include <message.hpp>


namespace deepstream
{
	namespace parser
	{
		struct Error
		{
			enum Tag
			{
				UNEXPECTED_TOKEN,
				UNEXPECTED_EOF,
				EMPTY_PAYLOAD
			};


			explicit Error(std::size_t offset, std::size_t length, Tag tag) :
				location_(offset, length),
				tag_(tag)
			{}

			Location location_;
			Tag tag_;
		};


		typedef deepstream_parser_state State;
	}
}


struct deepstream_parser_state
{
	typedef std::vector<deepstream::Message> MessageList;
	typedef std::vector<deepstream::parser::Error> ErrorList;

	explicit deepstream_parser_state(const char* buf) :
		buffer(buf),
		tokenizing_header(true),
		offset(0)
	{}

	const char* const buffer;
	bool tokenizing_header;
	std::size_t offset;

	MessageList messages;
	ErrorList errors;
};

#endif
