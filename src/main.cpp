
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
	test_rechner();

	//bmath::Term t1("sin(a+b)^2+cos(a+b)^2");
	bmath::Term t1("5-2+abs(3-4i)+e^(x*sin(a+b)^2+x*cos(a+b)^2)");
	std::cout << t1 << std::endl;
	t1.combine();
	std::cout << t1 << std::endl;

}
