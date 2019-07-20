#include "exceptions.h"

using namespace bmath;

XTermConstructionError::XTermConstructionError(const std::string& message_)
	:message(message_)
{
}

XTermConstructionError::~XTermConstructionError() throw()
{
	//no directly owned dynamic memory
}

const char* XTermConstructionError::what() const throw()
{
	return message.c_str();
}
