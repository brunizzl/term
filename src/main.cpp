
#include <iostream>
#include "test.h"
#include "term.h"
extern std::array<bmath::intern::Pattern, 1> patterns;

int main()
{
	//test_timing();
	//test_strings();
	//test_strings_2();
	//test_length();
	//test_function("atan(x)");
	//test_rechner();

	bmath::Term t1("a+1");
	t1.combine();
	std::cout << t1 << std::endl;
	std::cout << patterns[0].print() << std::endl;

}
