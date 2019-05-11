
#include <iostream>
#include "test.h"
#include "term.h"

int main()
{
	//test_timing();
	//test_strings();
	//test_strings_2();
	//test_length();
	//test_function("atan(x)");
	//test_rechner();
	bmath::Term t1 = bmath::sin1.first;
	bmath::Term t2("2+a");
	bmath::Term t3("20-5*b");
	std::cout << (t1 + t2) / t3 << std::endl;
}
