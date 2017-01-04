#include <message.hpp>

#include <cassert>


namespace deepstream
{

Message::Message(const char* p, Topic topic, Action action, bool isAck) :
	buffer_(p),
	topic_(topic),
	action_(action),
	isAck_(isAck)
{
	assert(p);
}

}
