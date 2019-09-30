
#include <iostream>
#include "test.h"
#include "term.h"

int main()
{
	try {
		//test_timing();
		test_strings();
		//test_function("atan(x)");
		//test_vergleich();
		test_rechner();
	}
	catch (bmath::XTermConstructionError err) {
		std::cout << err.what() << std::endl;
	}
}
