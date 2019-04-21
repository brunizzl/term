
#define _USE_MATH_DEFINES
#include <iostream>
#include "term.h"

#include "test.cpp"

int main()
{
	//test_strings();
	//test_strings_2();
	//test_length();
	//test_function("1/200*x^5");

	bmath::Term test("sin(5/2*pi*sqrt(4))");
	std::string pi = "pi";
	std::cout << test << '\n';
	test.search_and_replace(pi, M_PI);
	test.combine();
	std::cout << test << '\n';
}




