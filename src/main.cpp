
#include <iostream>
#include "test.h"
#include "term.h"
extern std::array<bmath::intern::Pattern*, 2> patterns;

int main()
{
	//test_timing();
	//test_strings();
	//test_strings_2();
	//test_length();
	//test_function("atan(x)");
	//test_rechner();

	bmath::Term t1("sin(a+b)^2+cos(a+b)^2");
	std::cout << t1 << std::endl;
	t1.match_and_transform(*patterns[0]);
	std::cout << t1 << std::endl;
	//std::cout << patterns[0]->print() << std::endl;

}
