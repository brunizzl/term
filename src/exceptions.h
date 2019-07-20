#pragma once

#include <exception>
#include <string>

namespace bmath {

	class XTermConstructionError : public std::exception
	{
	private:
		// error message 
		std::string message;
	public:

		XTermConstructionError(const std::string& message_);
		virtual ~XTermConstructionError() throw();

		const char* what() const throw();
	};

} //namespace bmath

