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

		//overloaded from std::exception
		const char* what() const throw();
	};

	class XTermCouldNotBeEvaluated : public std::exception
	{
	private:
		// name of variable preventing evaluation
		std::string message;
	public:
		XTermCouldNotBeEvaluated(const std::string& message_);

		//overloaded from std::exception
		const char* what() const throw();
	};

} //namespace bmath

