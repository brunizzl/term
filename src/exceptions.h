
#pragma once

#include <exception>

namespace bmath {

	class XTermConstructionError : public std::exception
	{
	private:
	public:
		XTermConstructionError(const char* const message);

		//overloaded from std::exception
		const char* what() const throw();
	};

	class XTermCouldNotBeEvaluated : public std::exception
	{
	private:
	public:
		//var_name is name of unknown variable, preventing evaluation
		XTermCouldNotBeEvaluated(const char* const var_name);

		//overloaded from std::exception
		const char* what() const throw();
	};

} //namespace bmath

