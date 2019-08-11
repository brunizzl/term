
#include <iostream>
#include "test.h"
#include "term.h"
extern std::array<bmath::intern::Pattern*, 2> patterns;

int main()
{
	//for (int c = 0; c < 256; c++) {
	//	std::cout << c << ' ' << static_cast<char>(c) << '\n';
	//}
	try {
		//test_timing();
		//test_strings();
		//test_strings_2();
		//test_length();
		//test_function("atan(x)");
		test_rechner();
	}
	catch (bmath::XTermConstructionError err) {
		std::cout << err.what() << std::endl;
	}
}
