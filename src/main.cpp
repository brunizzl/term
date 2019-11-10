
#include <iostream>
#include "term.h"
//#include "internal_functions.h"
//#include "arguments.h"
//#include "operations.h"
#include "test.h"

int main()
{
	try {
		test_strings();
		//print_all_patterns();
		//test_vergleich();
		test_rechner();
	}
	catch (bmath::XTermConstructionError err) {
		std::cout << err.what() << std::endl;
	}
}
