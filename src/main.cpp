
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

	bmath::Term test("ln(2) - LN[2]");
	std::string LN2("LN[2]");
	std::cout << test << '\n';
	test.search_and_replace(LN2, M_LN2);
	test.combine();
	std::cout << test << '\n';
}




