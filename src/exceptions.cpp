
#include "exceptions.h"

using namespace bmath;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//XTermConstructionError\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XTermConstructionError::XTermConstructionError(const char* const message)
	:std::exception(message)
{}

const char* XTermConstructionError::what() const throw()
{
	return std::exception::what();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//XTermCouldNotBeEvaluated\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XTermCouldNotBeEvaluated::XTermCouldNotBeEvaluated(const char* const var_name)
	:std::exception(var_name) 
{}

const char* XTermCouldNotBeEvaluated::what() const throw()
{
	return std::exception::what();
}
