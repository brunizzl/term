#include "exceptions.h"

using namespace bmath;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//XTermConstructionError\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XTermConstructionError::XTermConstructionError(const std::string& message_)
	:message(message_)
{
}

const char* XTermConstructionError::what() const throw()
{
	return message.c_str();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//XTermCouldNotBeEvaluated\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XTermCouldNotBeEvaluated::XTermCouldNotBeEvaluated(const std::string& message_)
	:message(message_)
{
}

const char* XTermCouldNotBeEvaluated::what() const throw()
{
	return message.c_str();
}
