
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
	bmath::Term test("a+b");
	try {
		std::cout << test.evaluate({ { "a", 1 }, { "b", {1, 1} } }) << std::endl;
	}
	catch (bmath::XTermCouldNotBeEvaluated err) {
		std::cout << err.what() << std::endl;
	}
	test_rechner();
}
